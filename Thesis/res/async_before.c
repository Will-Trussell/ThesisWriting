/**
 * "Main Loop": Loop until shutdown or restart is signalled.
 *
 * This function loops until a shutdown or restart of ngIRCd is signalled and
 * calls io_dispatch() to check for readable and writable sockets every second.
 * It checks for status changes on pending connections (e. g. when a hostname
 * has been resolved), checks for "penalties" and timeouts, and handles the
 * input buffers.
 */
GLOBAL void Conn_Handler(void) {
	int i;
	size_t wdatalen;
	struct timeval tv;
	time_t t;
	bool command_available;
	while (!NGIRCd_SignalQuit && !NGIRCd_SignalRestart) {
		t = time(NULL);
		command_available = false;
		//Utility checks omitted for brevity
		for (i = 0; i < Pool_Size; i++) { // Look for non-empty read buffers
			if ((My_Connections[i].sock > NONE)
			    && (array_bytes(&My_Connections[i].rbuf) > 0)) {
				Handle_Buffer(i); // handle the received data
			}
		}
		for (i = 0; i < Pool_Size; i++) { // Look for non-empty write buffers
			if (My_Connections[i].sock <= NONE)
				continue;
			wdatalen = array_bytes(&My_Connections[i].wbuf);
			if (wdatalen > 0)
			{
				//SSL Code omitted for brevity
				io_event_add(My_Connections[i].sock, IO_WANTWRITE);
			}
		}
		for (i = 0; i < Pool_Size; i++) { //Check sockets for readability
			if (My_Connections[i].sock <= NONE)
				continue;
			//SSL code omitted for brevity
			if (Proc_InProgress(&My_Connections[i].proc_stat)) {
				io_event_del(My_Connections[i].sock, IO_WANTREAD);//Wait on subprocesses
				continue;
			}
			if (Conn_OPTION_ISSET(&My_Connections[i], CONN_ISCONNECTING))
				continue; //Wait for connect() to complete
			if (My_Connections[i].delaytime > t) { //penalty set, ignore socket
				io_event_del(My_Connections[i].sock, IO_WANTREAD);
				continue;
			}
			if (array_bytes(&My_Connections[i].rbuf) >= COMMAND_LEN) {
				io_event_del(My_Connections[i].sock, IO_WANTREAD);
				command_available = true;
				continue;
			}
			io_event_add(My_Connections[i].sock, IO_WANTREAD);
		}
		tv.tv_usec = 0;
		tv.tv_sec = command_available ? 0 : 1;
		i = io_dispatch(&tv);
		if (i == -1 && errno != EINTR) { exit(1); } //fatal errors
		if (Conf_IdleTimeout > 0 && NumConnectionsAccepted > 0
		    && idle_t > 0 && time(NULL) - idle_t >= Conf_IdleTimeout) {
			NGIRCd_SignalQuit = true;
		}
	}
	//Server shutdown messages omitted
} /* Conn_Handler */
