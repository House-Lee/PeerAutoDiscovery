/*
 * queue.c
 *
 *  Created on: Jul 8, 2013
 *      Author: House_Lee
 */

#include <string.h>
#include "queue.h"

#define GET_QUEUE_ALLOC_SIZE(datasize) (sizeof(char*) + datasize)

Queue_t Queue_Init(int nDataSize) {
	if (nDataSize <= 0)
		return NULL;
	Queue_t new_queue = ylmalloc(sizeof(Queue_struct));
	new_queue->oplock = SpinLock_Init();
	new_queue->head_ = new_queue->tail_ = NULL;
	new_queue->n_cnt_ = 0;
	new_queue->n_datasize_ = nDataSize;
	return new_queue;
}
int Queue_Push(Queue_t queue , void* inData) {
	char* tmp;
	int n_alloc_size = GET_QUEUE_ALLOC_SIZE(queue->n_datasize_);
	tmp = ylmalloc(n_alloc_size);
	memcpy(tmp + sizeof(char*) , inData , queue->n_datasize_);
	*(char**)tmp = NULL;
	SpinLock_Lock(queue->oplock);
	if (!queue->n_cnt_) {
		queue->head_ = queue->tail_ = tmp;
	} else {
		*(char**)queue->tail_ = tmp;
		queue->tail_ = tmp;
	}
	++queue->n_cnt_;
	SpinLock_Unlock(queue->oplock);
	return 0;
}
int Queue_Pop(Queue_t queue , void* outData) {
	if (!outData) {
		return ERR_QUEUE_BUFFER_NULL;
	}
	char* tmp = NULL;
	int	nDataSize = 0;
	if (queue->n_cnt_) {
		SpinLock_Lock(queue->oplock);
		if (queue->n_cnt_) {
			tmp = queue->head_;
			nDataSize = queue->n_datasize_;
			queue->head_ = *(char**)tmp;
			if (!(--queue->n_cnt_)) {
				queue->tail_ = NULL;
			}
		} else {
			SpinLock_Unlock(queue->oplock);
			return ERR_QUEUE_EMPTY;
		}
		SpinLock_Unlock(queue->oplock);
	} else {
		return ERR_QUEUE_EMPTY;
	}
	memcpy(outData , tmp + sizeof(char*) , nDataSize);
	ylfree(tmp);
	return 0;
}
int Queue_GetHeadDataPtr(Queue_t queue , void** DataPtr) {
	if (!DataPtr) {
		return ERR_QUEUE_BUFFER_NULL;
	}
	if (queue->n_cnt_) {
		SpinLock_Lock(queue->oplock);
		if (queue->n_cnt_) {
			*DataPtr = queue->head_ + sizeof(char*);
		} else {
			SpinLock_Unlock(queue->oplock);
			return ERR_QUEUE_EMPTY;
		}
		SpinLock_Unlock(queue->oplock);
	} else {
		return ERR_QUEUE_EMPTY;
	}
	return 0;
}
int Queue_GetTailDataPtr(Queue_t queue, void** DataPtr) {
	if (!DataPtr) {
		return ERR_QUEUE_BUFFER_NULL;
	}
	if (queue->n_cnt_) {
		SpinLock_Lock(queue->oplock);
		if (queue->n_cnt_) {
			*DataPtr = queue->tail_ + sizeof(char*);
		} else {
			SpinLock_Unlock(queue->oplock);
			return ERR_QUEUE_EMPTY;
		}
		SpinLock_Unlock(queue->oplock);
	} else {
		return ERR_QUEUE_EMPTY;
	}
	return 0;
}
int Queue_Clear(Queue_t queue) {
	if (queue->n_cnt_) {
		SpinLock_Lock(queue->oplock);
		if (queue->n_cnt_) {
			while(queue->head_) {
				char* tmp = queue->head_;
				queue->head_ = *(char**)tmp;
				ylfree(tmp);
			}
			queue->head_ = queue->tail_ = NULL;
			queue->n_cnt_ = 0;
		} else {
			SpinLock_Unlock(queue->oplock);
			return 0;
		}
		SpinLock_Unlock(queue->oplock);
	} else {
		return 0;
	}
	return 0;
}
int Queue_Destroy(Queue_t queue) {
	Queue_Clear(queue);
	SpinLock_Destroy(queue->oplock);
	ylfree(queue);
	return 0;
}
