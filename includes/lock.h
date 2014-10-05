/*
 * lock.h
 *
 *  Created on: Jul 8, 2013
 *      Author: House_Lee
 */

#ifndef INCLUDES_LOCK_H_
#define INCLUDES_LOCK_H_

#include <stdlib.h>
#include <pthread.h>
#include "commual.h"
#include "memory.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct SPIN_LOCK {
	volatile int lock;
} SpinLock_struct;

typedef SpinLock_struct* SpinLock_t;

static inline void cpu_relax() {
	__asm__ __volatile__ (
			"rep;nop\n\t"
			:::"memory"
	);
}

static inline int _spinlock_islock(SpinLock_t lock) {
	return lock->lock;
}
static inline int _spinlock_trylock(SpinLock_t lock) {
	volatile int rtn = 0;
	__asm__ __volatile__ (
			"mfence;\n\t"
			"xorl %%eax,%%eax;\n\t"
			"movl $0,%%eax;\n\t"
			"movl $1,%%ecx;\n\t"
			"lock cmpxchgl %%ecx,%0;\n\t"
			"mfence;\n\t"
			"jnz MOO_SPIN_LOCK_FAILED;\n\t"
			"movl $1,%1;\n\t"
			"MOO_SPIN_LOCK_FAILED:\n\t"
			:"+m"(lock->lock),"+m"(rtn)
			:
			:"eax","ecx","memory"
	);
	return rtn;
}
static inline SpinLock_t SpinLock_Init(void) {
	SpinLock_t new_lock = (SpinLock_t)ylmalloc(sizeof(SpinLock_struct));
	new_lock->lock = false;
	return new_lock;
}
static inline void SpinLock_Lock(SpinLock_t lock) {
	do {
		while (_spinlock_islock(lock))
			cpu_relax();
	}while (!_spinlock_trylock(lock));
}
static inline void SpinLock_Unlock(SpinLock_t lock) {
	__asm__ __volatile__ (
			"mfence;\n\t"
			"movl $1,%%eax;\n\t"
			"movl $0,%%ecx;\n\t"
			"lock cmpxchgl %%ecx,%0;\n\t"
			"mfence;\n\t"
			:"+m"(lock->lock)
			:
			:"eax","ecx","memory"
	);
}

static inline void SpinLock_Destroy(SpinLock_t lock) {
	ylfree(lock);
}

typedef struct {
	volatile int is_writing_;
	volatile int writer_cnt_;
	volatile int reader_cnt_;
	volatile int write_owner_;
}WRLock_struct;
typedef WRLock_struct* WRLock_t;

WRLock_t 	WRLock_Init();
void		WRLock_LockR(WRLock_t lock);
void		WRLock_UnlockR(WRLock_t lock);
void		WRLock_LockW(WRLock_t lock);
void		WRLock_UnlockW(WRLock_t lock);
void		WRLock_Destroy(WRLock_t lock);

typedef struct {
	pthread_mutex_t lock_;
}StdMutex_struct;
typedef StdMutex_struct* StdMutex_t;

StdMutex_t	StdMutex_Init();
void 		StdMutex_Lock(StdMutex_t lock);
void 		StdMutex_Unlock(StdMutex_t lock);
void 		StdMutex_Destroy(StdMutex_t lock);


#ifdef	__cplusplus
}
#endif

#endif /* INCLUDES_LOCK_H_ */
