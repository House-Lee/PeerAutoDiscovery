/*
 * memory.h
 *
 *  Created on: Jul 8, 2013
 *      Author: House_Lee
 */

#ifndef INCLUDES_MEMORY_H_
#define INCLUDES_MEMORY_H_

#include <stddef.h>

#if defined(USE_JEMALLOC)
#define Yunlu_MALLOC_LIB ("jemalloc-" __xstr(JEMALLOC_VERSION_MAJOR) "." __xstr(JEMALLOC_VERSION_MINOR) "." __xstr(JEMALLOC_VERSION_BUGFIX))
#include <jemalloc/jemalloc.h>
#if (JEMALLOC_VERSION_MAJOR == 2 && JEMALLOC_VERSION_MINOR >= 1) || (JEMALLOC_VERSION_MAJOR > 2)
#define HAVE_MALLOC_SIZE 1
#define zmalloc_size(p) je_malloc_usable_size(p)
#else
#error "Newer version of jemalloc required"
#endif
#else
#define Yunlu_MALLOC_LIB "libc"
#endif

#ifdef	__cplusplus
extern "C" {
#endif

void* 	ylmalloc(size_t size);
void* 	ylcalloc(size_t size);
void* 	ylrealloc(void* ptr , size_t size);
void	ylfree(void* ptr);
char*	ylstrdup(const char* s);



#ifdef	__cplusplus
}
#endif

#endif /* INCLUDES_MEMORY_H_ */
