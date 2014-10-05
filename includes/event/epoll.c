/*
 * epoll.c
 *
 *  Created on: Jul 11, 2013
 *      Author: House_Lee
 */

#include <sys/epoll.h>

typedef struct EVAPI {
	int epfd;
	struct epoll_event* events;
}EventApi_struct;
typedef EventApi_struct* EventApi_t;

static int _evApiCreate(EventLoop_t evloop) {
	EventApi_t state = ylmalloc(sizeof(EventApi_struct));
	state->events = ylmalloc(sizeof(struct epoll_event) * evloop->setsize);
	state->epfd = epoll_create(1024);//1024 is just a hint
	if (state->epfd == -1) {
		ylfree(state->events);
		ylfree(state);
		return EV_ERR;
	}
	evloop->apidata = state;
	return EV_OK;
}

static void _evApiFree(EventLoop_t evloop) {
	EventApi_t state = (EventApi_t)evloop->apidata;
	close(state->epfd);
	ylfree(state->events);
	ylfree(state);
}

static int _evApiAddEvent(EventLoop_t evloop, int fd, int mask) {
	EventApi_t state = (EventApi_t)evloop->apidata;
	EventFile_t* ev = Map_GetPtr(evloop->events , &fd);
	if (ev == NULL) {
		return EV_ERR;
	}
	struct epoll_event ee;
	int op = (ev->mask == EV_NONE) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
	ee.events = 0;
	mask |= ev->mask;
	if (mask & EV_READABLE) {
		ee.events |= EPOLLIN;
	}
	if (mask & EV_WRITABLE) {
		ee.events |= EPOLLOUT;
	}
	ee.data.u64 = 0;//avoid valgrind warning
	ee.data.fd = fd;
	if (epoll_ctl(state->epfd , op , fd, &ee) == -1) {
		return EV_ERR;
	}
	return EV_OK;
}

static void _evApiDelEvent(EventLoop_t evloop , int fd, int delmask) {
	EventApi_t state = (EventApi_t)evloop->apidata;
	EventFile_t* ev = Map_GetPtr(evloop->events, &fd);
	if (ev == NULL) {
		return;
	}
	struct epoll_event ee;
	int mask = ev->mask & (~delmask);
	ee.events = 0;
	if (mask & EV_READABLE) {
		ee.events |= EPOLLIN;
	}
	if (mask & EV_WRITABLE) {
		ee.events |= EPOLLOUT;
	}
	ee.data.u64 = 0; //avoid valgrind warning
	ee.data.fd = fd;
	if (mask != EV_NONE) {
		epoll_ctl(state->epfd,EPOLL_CTL_MOD,fd,&ee);
	} else {
		epoll_ctl(state->epfd,EPOLL_CTL_DEL,fd,&ee);
	}
}

static int _evApiLoop(EventLoop_t evloop , struct timeval *tvp) {
	EventApi_t state = (EventApi_t)evloop->apidata;
	int rtn, nevents = 0;
	rtn = epoll_wait(state->epfd , state->events , evloop->setsize, tvp?(tvp->tv_sec*1000 + tvp->tv_usec/1000):-1);
	if (rtn > 0) {
		nevents = rtn;
		for (int j = 0; j != nevents; ++j) {
			int mask = 0;
			struct epoll_event *e = &state->events[j];
			if (e->events & EPOLLIN) {
				mask |= EV_READABLE;
			}
			if ((e->events & EPOLLOUT) || (e->events & EPOLLERR) || (e->events & EPOLLHUP)) {
				mask |= EV_WRITABLE;
			}
			evloop->evbuffers[j].fd = e->data.fd;
			evloop->evbuffers[j].mask = mask;
		}
	}
	return nevents;
}

static char* _evApiName(void) {
	return "epoll";
}




















