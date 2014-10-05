/*
 * util.h
 *
 *  Created on: Jul 9, 2013
 *      Author: House_Lee
 */

#ifndef INCLUDES_UTIL_H_
#define INCLUDES_UTIL_H_

#include "commual.h"

#ifdef	__cplusplus
extern "C" {
#endif

static inline int atomic_fetch_then_add(int* src, int value) {
	//return *src
	__asm __volatile(
			"lock; xaddl %%eax,%2;\n\t"
			:"=a"(value)
			:"a"(value),"m"(*src)
			:"memory"
	);
	return value;
}
static inline int atomic_add_then_fetch(int* src, int value) {
	//return (*src) + value
	__asm __volatile(
			"lock; xaddl %%eax,%2;\n\t"
			:"=a"(value)
			:"a"(value),"m"(*src)
			:"memory"
	);
	return *src;
}
static inline int atomic_fetch_then_sub(int* src, int value) {
	//return *src
	__asm __volatile(
			"lock; xaddl %%eax,%2;\n\t"
			:"=a"(value)
			:"a"(-value),"m"(*src)
			:"memory"
	);
	return value;
}
static inline int atomic_sub_then_fetch(int* src, int value) {
	//return (*src) - value
	__asm __volatile(
			"lock; xaddl %%eax,%2;\n\t"
			:"=a"(value)
			:"a"(-value),"m"(*src)
			:"memory"
	);
	return *src;
}
static inline bool atomic_compare_and_swap_bool(int* src , int cmp_value , int swap_value) {
	int rtn = 0;
	__asm __volatile(
			"lock; cmpxchgl %%ecx,%1;\n\t"
			"jnz SOAR_ATOMIC_CASB_END;\n\t"
			"movl $1,%0;\n\t"
			"SOAR_ATOMIC_CASB_END:\n\t"
			:"=m"(rtn),"+m"(*src)
			:"a"(cmp_value),"c"(swap_value)
			:"memory"
	);
	return rtn;
}
static inline int atomic_compare_and_swap_value(int* src , int cmp_value , int swap_value) {
	int rtn = *src;
	__asm __volatile(
			"lock; cmpxchgl %%ecx,%0;\n\t"
			:"=m"(*src)
			:"a"(cmp_value),"c"(swap_value)
			:"memory"
	);
	return rtn;
}

bool is_numeric(const char* str);

int get_thread_id();

#ifdef	__cplusplus
}
#endif

#endif /* INCLUDES_UTIL_H_ */
