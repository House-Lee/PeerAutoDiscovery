/*
 * map.h
 *
 *  Created on: Jul 11, 2013
 *      Author: House_Lee
 */

#ifndef MAP_H_
#define MAP_H_

#include "../algorithm/rbtree.h"
#include "lock.h"

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Function to compare the key,should return:
 * any negative integer if left_key < right_key;
 * 0 if left_key == right_key;
 * any positive integer if left_key > right_key;
 **/
typedef int (*mapKeyCmpFunc)(const void*, const void*);
typedef void* (*mapKeyDupFunc)(const void*);
typedef void (*mapKeyFreeFunc)(void*);
typedef void* (*mapValDupFunc)(const void*);
typedef void (*mapValFreeFunc)(void*);
typedef void  (*foreach_call_back)(const void* key,const void* data , void* args);

typedef struct MAP {
	RBTree_t		map_store;
	volatile int	cur_size;
	WRLock_t		oplock;
	mapKeyCmpFunc	key_cmp_func;
	mapKeyDupFunc	key_dup_func;
	mapKeyFreeFunc	key_free_func;
	mapValDupFunc	val_dup_func;
	mapValFreeFunc	val_free_func;
}Map_struct;
typedef Map_struct* Map_t;

Map_t	Map_Init(mapKeyCmpFunc,mapKeyDupFunc,mapKeyFreeFunc,mapValDupFunc,mapValFreeFunc);
int		Map_Set(Map_t map_set,const void* key, const void* data);
void*	Map_SetAndGetPtr(Map_t map_set , const void* key, const void* data);
void*	Map_Get(Map_t map_set, const void* key);
void*	Map_GetPtr(Map_t map_set, const void* key);
void*	Map_GetAndDel(Map_t map_set , void* key);
int		Map_GetSize(Map_t map_set);
void	Map_FreeResult(Map_t map_set, void* data);
int		Map_Del(Map_t map_set, void* key);
void	Map_Foreach(Map_t map_set , foreach_call_back callback , void* args);
void	Map_Destroy(Map_t map_set);

int 	Map_DefaultIntCmpFunc(const void* key_left, const void* key_right);
int 	Map_DefaultStringCmpFunc(const void* key_left , const void* key_right);
void* 	Map_DefaultIntDupFunc(const void* src);
void* 	Map_DefaultStringDupFunc(const void* src);
void  	Map_DefaultFreeFunc(void* ptr);

#ifdef	__cplusplus
}
#endif

#endif /* MAP_H_ */
