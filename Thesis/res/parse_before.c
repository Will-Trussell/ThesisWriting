GLOBAL bool Parse_Request( CONN_ID Idx, char *Request ){
	REQUEST req;
	char *start, *ptr;
	bool closed;

//Code omitted for brevity

	if (Request[0] == ':') {
		req.prefix = Request + 1;
		ptr = strchr( Request, ' ' );
		if( ! ptr ) {
			return Conn_WriteStr(Idx, "ERROR :Prefix without command");
		}
		*ptr = '\0';
		start = ptr + 1;
	} else start = Request;
	ptr = strchr( start, ' ' );
	if( ptr ) {
		*ptr = '\0';
	}
	req.command = start;
	if( ptr ) {
		start = ptr + 1;
		while( start ) {
			if( start[0] == ':' ) {
				req.argv[req.argc] = start + 1;
				ptr = NULL;
			} else {
				req.argv[req.argc] = start;
				ptr = strchr( start, ' ' );
				if( ptr ) {
					*ptr = '\0';
				}
			}
			req.argc++;
			if( start[0] == ':' ) break;
			if( req.argc > 14 ) break;
			if( ptr ) start = ptr + 1;
			else start = NULL;
		}
	}
	if(!Validate_Prefix(Idx, &req, &closed))
		return !closed;
	if(!Validate_Command(Idx, &req, &closed))
		return !closed;
	if(!Validate_Args(Idx, &req, &closed))
		return !closed;

	return Handle_Request(Idx, &req);
} /* Parse_Request */
