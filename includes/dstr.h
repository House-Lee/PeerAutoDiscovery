/*
 * dstr.h - Dynamic String
 *
 *  Created on: Jul 18, 2013
 *      Author: House_Lee
 */

#ifndef INCLUDES_DSTR_H_
#define INCLUDES_DSTR_H_

#include <sys/types.h>

typedef char* DStr_t;

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct dstr_struct {
	int len;
	int free;
	char buf[];
}DStr_struct;

static inline size_t DStrlen(const DStr_t s) {
	DStr_struct* st = (DStr_struct*)(s - sizeof(DStr_struct));
	return st->len;
}
static inline size_t DStrAvailLen(const DStr_t s) {
	DStr_struct* st = (DStr_struct*) (s - sizeof(DStr_struct));
	return st->free;
}

DStr_t DStrNew(const char* init);
DStr_t DStrNewEmpty(int hint);
DStr_t DStrAppend(DStr_t s , const char* t);
DStr_t DStrNAppend(DStr_t s, const char* t, size_t nlen);
DStr_t DStrAssign(DStr_t s, const char* t);
DStr_t DStrNAssign(DStr_t s, const char* t, size_t nlen);
void   DStrClear(DStr_t s);
void   DStrFree(DStr_t s);

#ifdef	__cplusplus
}
#endif

#endif /* INCLUDES_DSTR_H_ */
