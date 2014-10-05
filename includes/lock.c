/*
 * lock.c
 *
 *  Created on: Jul 8, 2013
 *      Author: House_Lee
 */

#include <string.h>
#include <unistd.h>

#include "lock.h"
#include "util.h"

#define MAX_TRY_TIME 1000000


WRLock_t WRLock_Init() {
	WRLock_t new_lock = (WRLock_t)ylmalloc(sizeof(WRLock_struct));
	memset(new_lock , 0 , sizeof(WRLock_struct));
	return new_lock;
}

void WRLock_LockR(WRLock_t lock) {
	int try_time = 0;
	while (!atomic_compare_and_swap_bool((int*)&lock->writer_cnt_, 0, 0)) {
		if (try_time == MAX_TRY_TIME) {
			usleep(1000);
		} else {
			++try_time;
		}
	}
	while (atomic_add_then_fetch((int*)&lock->reader_cnt_, 1) <= 0);
}
void WRLock_UnlockR(WRLock_t lock) {
	atomic_fetch_then_sub((int*)&lock->reader_cnt_ , 1);
}
void WRLock_LockW(WRLock_t lock) {
	int thread_id = get_thread_id();
	while(atomic_add_then_fetch((int*)&lock->writer_cnt_ , 1) <= 0);
	int try_time = 0;
	while (!atomic_compare_and_swap_bool((int*)&lock->reader_cnt_, 0, 0)
			|| !atomic_compare_and_swap_bool((int*)&lock->is_writing_, 0, 1)) {
		if (try_time < MAX_TRY_TIME) {
			++try_time;
			cpu_relax();
		} else {
			usleep(1000);
		}
	}
	atomic_compare_and_swap_bool((int*)&lock->write_owner_ , (int)lock->write_owner_ , thread_id);
}
void WRLock_UnlockW(WRLock_t lock) {
	atomic_compare_and_swap_bool((int*)&lock->write_owner_ , (int)lock->write_owner_ , 0);
	atomic_fetch_then_sub((int*)&lock->writer_cnt_ , 1);
	atomic_compare_and_swap_value((int*)&lock->is_writing_ , 1 , 0);
}
void WRLock_Destroy(WRLock_t lock){
	ylfree(lock);
}


StdMutex_t StdMutex_Init() {
	StdMutex_t new_lock = (StdMutex_t)ylmalloc(sizeof(StdMutex_struct));
	pthread_mutex_init(&new_lock->lock_ , NULL);
	return new_lock;
}
void StdMutex_Lock(StdMutex_t lock) {
	pthread_mutex_lock(&lock->lock_);
}
void StdMutex_Unlock(StdMutex_t lock) {
	pthread_mutex_unlock(&lock->lock_);
}
void StdMutex_Destroy(StdMutex_t lock) {
	pthread_mutex_destroy(&lock->lock_);
	ylfree(lock);
}




















