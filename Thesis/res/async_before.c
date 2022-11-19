static bool
io_event_change_epoll(int fd, short what, const int action)
{
	struct epoll_event ev = { 0, {0} };
	ev.data.fd = fd;

	if (what & IO_WANTREAD)
		ev.events = EPOLLIN | EPOLLPRI;
	if (what & IO_WANTWRITE)
		ev.events |= EPOLLOUT;

	return epoll_ctl(io_masterfd, action, fd, &ev) == 0;
}

static int
io_dispatch_epoll(struct timeval *tv)
{
	time_t sec = tv->tv_sec * 1000;
	int i, ret, timeout = tv->tv_usec + sec;
	struct epoll_event epoll_ev[MAX_EVENTS];
	short type;

	if (timeout < 0)
		timeout = 1000;

	ret = epoll_wait(io_masterfd, epoll_ev, MAX_EVENTS, timeout);

	for (i = 0; i < ret; i++) {
		type = 0;
		if (epoll_ev[i].events & (EPOLLERR | EPOLLHUP))
			type = IO_ERROR;

		if (epoll_ev[i].events & (EPOLLIN | EPOLLPRI))
			type |= IO_WANTREAD;

		if (epoll_ev[i].events & EPOLLOUT)
			type |= IO_WANTWRITE;

		io_docallback(epoll_ev[i].data.fd, type);
	}

	return ret;
}

static void
io_library_init_epoll(unsigned int eventsize)
{
	int ecreate_hint = (int)eventsize;
	if (ecreate_hint <= 0)
		ecreate_hint = 128;
	io_masterfd = epoll_create(ecreate_hint);
	if (io_masterfd >= 0) {
		library_initialized = true;
		Log(LOG_INFO,
		    "IO subsystem: epoll (hint size %d, initial maxfd %u, masterfd %d).",
		    ecreate_hint, eventsize, io_masterfd);
		return;
	}
}

bool
io_event_create(int fd, short what, void (*cbfunc) (int, short))
{
	bool ret;
	io_event *i;

	assert(fd >= 0);

	i = (io_event *) array_alloc(&io_events, sizeof(io_event), (size_t) fd);
	if (!i) {
		Log(LOG_WARNING,
		    "array_alloc failed: could not allocate space for %d io_event structures",
		    fd);
		return false;
	}

	i->callback = cbfunc;
	i->what = 0;
	ret = io_event_change_epoll(fd, what, EPOLL_CTL_ADD);
	if (ret)
		i->what = what;
	return ret;
}

bool
io_event_add(int fd, short what)
{
	io_event *i = io_event_get(fd);

	if (!i) return false;

	if ((i->what & what) == what) /* event type is already registered */
		return true;

	io_debug("io_event_add: fd, what", fd, what);

	i->what |= what;
	if (io_masterfd >= 0)
		return io_event_change_epoll(fd, i->what, EPOLL_CTL_MOD);
	return false;
}


bool
io_close(int fd)
{
	io_event *i;

	i = io_event_get(fd);
	io_event_change_epoll(fd, 0, EPOLL_CTL_DEL);
	if (i) {
		i->callback = NULL;
		i->what = 0;
	}
	return close(fd) == 0;
}

bool
io_event_del(int fd, short what)
{
	io_event *i = io_event_get(fd);

	if (!i) return false;

	if (!(i->what & what)) /* event is already disabled */
		return true;

	i->what &= ~what;
	if (io_masterfd >= 0)
		return io_event_change_epoll(fd, i->what, EPOLL_CTL_MOD);
	return false;
}

int
io_dispatch(struct timeval *tv)
{
	if (io_masterfd >= 0)
		return io_dispatch_epoll(tv);
	return -1;
}

/* call the callback function inside the struct matching fd */
static void
io_docallback(int fd, short what)
{
	io_event *i = io_event_get(fd);

	io_debug("io_docallback; fd, what", fd, what);

	if (i->callback) {	/* callback might be NULL if a previous callback function
				   called io_close on this fd */
		i->callback(fd, (what & IO_ERROR) ? i->what : what);
	}
}
