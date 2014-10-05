/*
 * event.h
 *
 *  Created on: Jul 11, 2013
 *      Author: House_Lee
 */

#ifndef INCLUDES_EVENT_H_
#define INCLUDES_EVENT_H_

#include "map.h"

#ifdef	__cplusplus
extern "C" {
#endif

enum EV_STATUS {
	EV_ERR = -1,
	EV_OK = 0,
};

enum EV_ATTR {
	EV_NONE = 0,
	EV_READABLE = 1,
	EV_WRITABLE = 2
};
//disable the not-used warning during the compilation
#define EV_NOUSED(V) ((void) V)

struct EventLoop_struct;

typedef void (*evFunc)(struct EventLoop_struct* evloop , int fd, void* evData , int mask);
typedef void (*evBeforeLoopFunc)(struct EventLoop_struct* evloop);

typedef struct EventFile_t {
	int 	mask;// EV_READABLE or EV_WRITABLE
	evFunc	read_func;
	evFunc	write_func;
	void*	evdata;
}EventFile_t;

typedef struct EventBuf_t {
	int fd;
	int mask;
}EventBuf_t;

typedef struct EventLoop_struct {
	int 				setsize;
//	EventFile_t* 		events;
	Map_t				events;
	EventBuf_t*			evbuffers;
	evBeforeLoopFunc	beforeloop;
	void*				apidata;
	int					stop;
}EventLoop_struct;
typedef EventLoop_struct* EventLoop_t;

//Declarations
EventLoop_t Event_CreateLoop(int set_size);
void Event_DestroyLoop(EventLoop_t evloop);
void Event_Stop(EventLoop_t evloop);
int Event_CreateFileEvent(EventLoop_t evloop, int fd, int mask, evFunc proc, void* evdata);
void Event_DeleteFileEvent(EventLoop_t evloop, int fd, int mask);
int Event_GetFileEventMask(EventLoop_t evloop, int fd);
int Event_Process(EventLoop_t evloop);
void Event_Main(EventLoop_t evloop);
char* Event_GetApiName(void);
void Event_SetBeforeLoopFunc(EventLoop_t evloop, evBeforeLoopFunc func);


#ifdef	__cplusplus
}
#endif

#endif /* INCLUDES_EVENT_H_ */






















