/*
 * event.c
 *
 *  Created on: Jul 11, 2013
 *      Author: House_Lee
 */

#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "event.h"
#include "memory.h"
#include "commual.h"

#include "event/epoll.c"

static void* _evDupFunc(const void* ev) {
	EventFile_t* new_file_ev = ylmalloc(sizeof(EventFile_t));
	memcpy(new_file_ev , ev , sizeof(EventFile_t));
	return new_file_ev;
}

EventLoop_t Event_CreateLoop(int set_size) {
	if (!set_size) {
		return NULL;
	}
	EventLoop_t evloop = ylmalloc(sizeof(EventLoop_struct));
	evloop->evbuffers = ylmalloc(sizeof(EventBuf_t)*set_size);
	evloop->events = Map_Init(
								Map_DefaultIntCmpFunc,
								Map_DefaultIntDupFunc,
								Map_DefaultFreeFunc,
								_evDupFunc,
								Map_DefaultFreeFunc
								);
	evloop->setsize = set_size;
	evloop->stop = false;
	evloop->beforeloop = NULL;
	if (_evApiCreate(evloop) != EV_OK) {
		ylfree(evloop->evbuffers);
		Map_Destroy(evloop->events);
		ylfree(evloop);
		return NULL;
	}
	return evloop;
}

void Event_DestroyLoop(EventLoop_t evloop) {
	_evApiFree(evloop);
	ylfree(evloop->evbuffers);
	Map_Destroy(evloop->events);
	ylfree(evloop);
}

void Event_Stop(EventLoop_t evloop) {
	evloop->stop = true;
}

int Event_CreateFileEvent(EventLoop_t evloop, int fd, int mask, evFunc proc, void* evdata) {
	EventFile_t* fe = Map_GetPtr(evloop->events , &fd);
	if (fe == NULL) {
		EventFile_t tmp_fe;
		tmp_fe.mask = EV_NONE;
		Map_Set(evloop->events , &fd , &tmp_fe);
		fe = Map_GetPtr(evloop->events , &fd);
	}
	if (_evApiAddEvent(evloop , fd , mask) == EV_ERR) {
		return EV_ERR;
	}
	fe->mask |= mask;
	if (mask & EV_READABLE) {
		fe->read_func = proc;
	}
	if (mask & EV_WRITABLE) {
		fe->write_func = proc;
	}
	fe->evdata = evdata;
	return EV_OK;
}
void Event_DeleteFileEvent(EventLoop_t evloop, int fd, int mask) {
	EventFile_t* fe = Map_GetPtr(evloop->events , &fd);
	if (fe == NULL) {
		return;
	}
	if (fe->mask == EV_NONE)
		return;
	fe->mask = fe->mask & (~mask);
	_evApiDelEvent(evloop , fd , mask);
}
int Event_GetFileEventMask(EventLoop_t evloop, int fd) {
	EventFile_t* fe = Map_GetPtr(evloop->events , &fd);
	if (fe == NULL)
		return 0;
	return fe->mask;
}

int Event_Process(EventLoop_t evloop) {
	int nprocessed = 0;
	int nevents;
	nevents = _evApiLoop(evloop, NULL);
	for (int i = 0; i < nevents; ++i) {
		EventFile_t* fe = Map_GetPtr(evloop->events, &evloop->evbuffers[i].fd);
		if (fe == NULL) {
			return nprocessed;
		}
		int mask = evloop->evbuffers[i].mask;
		int fd = evloop->evbuffers[i].fd;
		int is_read = 0;
		if (fe->mask & mask & EV_READABLE) {
			is_read = 1;
			fe->read_func(evloop, fd, fe->evdata, mask);
		}
		if (fe->mask & mask & EV_WRITABLE) {
			if (!is_read || fe->write_func != fe->read_func) {
				fe->write_func(evloop, fd, fe->evdata, mask);
			}
		}
		++nprocessed;
	}
	return nprocessed;
}

void Event_Main(EventLoop_t evloop) {
	evloop->stop = 0;
	while(!evloop->stop) {
		if (evloop->beforeloop != NULL) {
			evloop->beforeloop(evloop);
		}
		Event_Process(evloop);
	}
}

char* Event_GetApiName(void) {
	return _evApiName();
}

void Event_SetBeforeLoopFunc(EventLoop_t evloop, evBeforeLoopFunc func) {
	evloop->beforeloop = func;
}















