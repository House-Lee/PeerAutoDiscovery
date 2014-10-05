/*
 * dstr.c
 *
 *  Created on: Jul 18, 2013
 *      Author: House_Lee
 */

#include <string.h>
#include "dstr.h"
#include "../includes/memory.h"

#define MAX_PRE_ALLOC 10240

static DStr_t _create_new_str(const char* init_str , size_t init_len) {
	DStr_struct* st;
	st = ylmalloc(sizeof(DStr_struct) + init_len + 1);
	st->len = init_len;
	st->free = 0;
	if (init_len && init_str) {
		memcpy(st->buf , init_str , init_len);
	}
	st->buf[init_len] = '\0';
	return (DStr_t)st->buf;
}

DStr_t DStrNew(const char* init) {
	size_t init_len = (init)?strlen(init):0;
	return _create_new_str(init , init_len);
}
DStr_t DStrNewEmpty(int hint) {
	DStr_struct* st;
	st = ylmalloc(sizeof(DStr_struct) + hint + 1);
	st->len = 0;
	st->free = hint;
	st->buf[0] = '\0';
	return (DStr_t) st->buf;
}

void DStrClear(DStr_t s) {
	DStr_struct* st = (DStr_struct*)(s - sizeof(DStr_struct));
	st->free += st->len;
	st->len = 0;
	st->buf[0] = '\0';
}

static DStr_t _make_room_(DStr_t s , size_t append_len) {
	size_t free = DStrAvailLen(s);
	if (free >= append_len) {
		return s;
	}
	int curlen = DStrlen(s);
	int newlen = curlen + append_len;
	if (newlen < MAX_PRE_ALLOC) {
		newlen *= 2;
	} else {
		newlen += MAX_PRE_ALLOC;
	}
	DStr_struct* oldds = (DStr_struct*)(s - sizeof(DStr_struct));
	DStr_struct* newds = ylrealloc(oldds , sizeof(DStr_struct) + newlen + 1);
	newds->free = newlen - curlen;
	return (DStr_t)newds->buf;
}
DStr_t DStrNAppend(DStr_t s, const char* t, size_t nlen) {
	DStr_struct* st;
	s = _make_room_(s , nlen);
	st = (DStr_struct*)(s - sizeof(DStr_struct));
	memcpy(s + st->len , t , nlen);
	st->len += nlen;
	st->free -= nlen;
	s[st->len] = '\0';
	return s;
}

DStr_t DStrAppend(DStr_t s , const char* t) {
	return DStrNAppend(s , t ,strlen(t));
}

DStr_t DStrNAssign(DStr_t s, const char* t, size_t nlen) {
	DStr_struct* st = (DStr_struct*)(s - sizeof(DStr_struct));
	int total_avaliable = st->len + st->free;
	if (total_avaliable < nlen) {
		s = _make_room_(s , nlen - st->len);
		st = (DStr_struct*)(s - sizeof(DStr_struct));
		total_avaliable = st->len + st->free;
	}
	memcpy(s , t , nlen);
	s[nlen] = '\0';
	st->len = nlen;
	st->free = total_avaliable - nlen;
	return s;
}

DStr_t DStrAssign(DStr_t s, const char* t) {
	return DStrNAssign(s , t , strlen(t));
}

void DStrFree(DStr_t s) {
	ylfree(s - sizeof(DStr_struct));
}
