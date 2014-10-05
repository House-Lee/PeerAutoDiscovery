/*
 * queue.h
 *
 *  Created on: Jul 8, 2013
 *      Author: House_Lee
 */

#ifndef INCLUDES_QUEUE_H_
#define INCLUDES_QUEUE_H_

#include "lock.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define QUEUE_OK 0

typedef struct _ULIM_QUEUE_ {
	char*		head_;
	char*		tail_;
	int			n_datasize_;
	int			n_cnt_;
	SpinLock_t 	oplock;
} Queue_struct;
typedef Queue_struct* Queue_t;

Queue_t Queue_Init(int nDataSize);
int Queue_Push(Queue_t queue , void* inData);
int Queue_Pop(Queue_t queue , void* outData);
int Queue_GetHeadDataPtr(Queue_t queue , void** DataPtr);
int Queue_GetTailDataPtr(Queue_t queue , void** DataPtr);
static inline int Queue_GetDataSize(Queue_t queue) {
	return (queue)?queue->n_datasize_:-1;
}
static inline int Queue_GetSize(Queue_t queue) {
	return (queue)?queue->n_cnt_:-1;
}
int Queue_Clear(Queue_t queue);
int Queue_Destroy(Queue_t queue);

#ifdef	__cplusplus
}
#endif

#endif /* INCLUDES_QUEUE_H_ */
