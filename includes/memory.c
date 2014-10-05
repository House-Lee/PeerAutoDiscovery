/*
 * memory.c
 *
 *  Created on: Jul 8, 2013
 *      Author: House_Lee
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

#if defined(USE_JEMALLOC)
#define t_malloc(size) je_malloc(size)
#define t_calloc(count,size) je_calloc(count,size)
#define t_realloc(ptr,size) je_realloc(ptr,size)
#define t_free(ptr) je_free(ptr)
#else
#define t_malloc(size) malloc(size)
#define t_calloc(count,size) calloc(count,size)
#define t_realloc(ptr,size) realloc(ptr,size)
#define t_free(ptr) free(ptr)
#endif

#define PREFIX_SIZE sizeof(size_t)

static void ylmalloc_oom (size_t size) {
	fprintf(stderr, "ylmalloc: Out of memory trying to allocate %zu bytes\n",
			size);
	fflush(stderr);
	abort();
}

void* ylmalloc(size_t size) {
	void* ptr = t_malloc(size + PREFIX_SIZE);
	if (!ptr) {
		ylmalloc_oom(size);
	}
	*((size_t*)ptr) = size;
	return (void*)((char*)ptr + PREFIX_SIZE);
}
void* 	ylcalloc(size_t size) {
	void* ptr = t_calloc(1 , size + PREFIX_SIZE);
	if (!ptr) {
		ylmalloc_oom(size);
	}
	*((size_t*) ptr) = size;
	return (void*) ((char*) ptr + PREFIX_SIZE);
}
void* 	ylrealloc(void* ptr , size_t size) {
	void* oldptr;
	void* newptr;
	oldptr = (char*)ptr - PREFIX_SIZE;
	newptr = t_realloc(oldptr , size + PREFIX_SIZE);
	if (!newptr) {
		ylmalloc_oom(size);
	}
	*((size_t*)newptr) = size;
	return (void*) ((char*) newptr + PREFIX_SIZE);
}
void	ylfree(void* ptr) {
	if (!ptr) {
		return;
	}
	void* realptr = (char*)ptr - PREFIX_SIZE;
	t_free(realptr);
}
char*	ylstrdup(const char* s) {
	size_t l = strlen(s) + 1;
	char* p = ylmalloc(l);
	memcpy(p , s , l);
	return p;
}
