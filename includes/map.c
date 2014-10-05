/*
 * map.c
 *
 *  Created on: Jul 11, 2013
 *      Author: House_Lee
 */

#include <string.h>

#include "map.h"
#include "memory.h"
#include "commual.h"
#include "queue.h"

int Map_DefaultIntCmpFunc(const void* key_left, const void* key_right) {
	return *(int*) key_left - *(int*) key_right;
}
int Map_DefaultStringCmpFunc(const void* key_left , const void* key_right) {
	return strcmp((const char*)key_left , (const char*)key_right);
}
void* Map_DefaultIntDupFunc(const void* src) {
	int* new_int = ylmalloc(sizeof(int));
	*new_int = *((int*)src);
	return (void*)new_int;
}
void* Map_DefaultStringDupFunc(const void* src) {
	char* new_str = ylstrdup((const char*) src);
	return new_str;
}
void  Map_DefaultFreeFunc(void* ptr) {
	ylfree(ptr);
}
Map_t Map_Init(	mapKeyCmpFunc keycmp,
				mapKeyDupFunc keydup,
				mapKeyFreeFunc keyfree,
				mapValDupFunc valdup,
				mapValFreeFunc valfree) {
	Map_t new_map = ylmalloc(sizeof(Map_struct));
	new_map->cur_size = 0;
	new_map->oplock = WRLock_Init();
	new_map->key_cmp_func = (keycmp == NULL)?Map_DefaultIntCmpFunc:keycmp;
	new_map->key_dup_func = (keydup == NULL)?Map_DefaultIntDupFunc:keydup;
	new_map->key_free_func = (keyfree == NULL)?Map_DefaultFreeFunc:keyfree;
	new_map->val_dup_func = (valdup == NULL)?Map_DefaultStringDupFunc:valdup;
	new_map->val_free_func = (valfree == NULL)?Map_DefaultFreeFunc:valfree;
	new_map->map_store = rbt_initRBTree(new_map->key_cmp_func , ylmalloc,ylfree);
	return new_map;
}
int	Map_Set(Map_t map_set,const void* key, const void* data) {
	RBT_node* result;
	WRLock_LockW(map_set->oplock);
	result = rbt_searchNodeByKey(map_set->map_store, key);
	if (result != NULL) {
		map_set->val_free_func(result->value);
		result->value = map_set->val_dup_func(data);
	} else {
		++(map_set->cur_size);
		void* new_key = map_set->key_dup_func(key);
		void* new_data = map_set->val_dup_func(data);
		if (rbt_insertNode(map_set->map_store, new_key, new_data)) {
			WRLock_UnlockW(map_set->oplock);
			map_set->key_free_func(new_key);
			map_set->val_free_func(new_data);
			return ERR_MAP_SET_EXCEPTION;
		}
	}
	WRLock_UnlockW(map_set->oplock);
	return 0;
}

void* Map_SetAndGetPtr(Map_t map_set , const void* key, const void* data) {
	void* rtn = NULL;
	RBT_node* result;
	WRLock_LockW(map_set->oplock);
	result = rbt_searchNodeByKey(map_set->map_store, key);
	if (result != NULL) {
		map_set->val_free_func(result->value);
		result->value = map_set->val_dup_func(data);
		rtn = result->value;
	} else {
		++(map_set->cur_size);
		void* new_key = map_set->key_dup_func(key);
		void* new_data = map_set->val_dup_func(data);
		rtn = new_data;
		if (rbt_insertNode(map_set->map_store, new_key, new_data)) {
			WRLock_UnlockW(map_set->oplock);
			map_set->key_free_func(new_key);
			map_set->val_free_func(new_data);
			return NULL;
		}
	}
	WRLock_UnlockW(map_set->oplock);
	return rtn;
}
void* Map_Get(Map_t map_set, const void* key) {
	RBT_node* result;
	WRLock_LockR(map_set->oplock);
	result = rbt_searchNodeByKey(map_set->map_store , key);
	if (result == NULL) {
		WRLock_UnlockR(map_set->oplock);
		return NULL;
	}
	void* rtn =  map_set->val_dup_func(result->value);
	WRLock_UnlockR(map_set->oplock);
	return rtn;
}
void* Map_GetAndDel(Map_t map_set , void* key) {
	RBT_node* result;
	WRLock_LockW(map_set->oplock);
	result = rbt_searchNodeByKey(map_set->map_store, key);
	if (result == NULL) {
		WRLock_UnlockW(map_set->oplock);
		return NULL;
	}
	void* rtn = map_set->val_dup_func(result->value);
	--(map_set->cur_size);
	void* d_key = result->key;
	void* d_data = result->value;
	rbt_deleteNode(map_set->map_store, key);
	WRLock_UnlockW(map_set->oplock);
	map_set->key_free_func(d_key);
	map_set->val_free_func(d_data);
	return rtn;
}
void* Map_GetPtr(Map_t map_set, const void* key) {
	RBT_node* result;
	WRLock_LockR(map_set->oplock);
	result = rbt_searchNodeByKey(map_set->map_store, key);
	if (result == NULL){
		WRLock_UnlockR(map_set->oplock);
		return NULL;
	}
	void* rtn = result->value;
	WRLock_UnlockR(map_set->oplock);
	return rtn;
}
int	Map_GetSize(Map_t map_set) {
	WRLock_LockR(map_set->oplock);
	int rtn = map_set->cur_size;
	WRLock_UnlockR(map_set->oplock);
	return rtn;
}
void Map_FreeResult(Map_t map_set, void* data) {
	if (data == NULL)
		return;
	map_set->val_free_func(data);
}
int	Map_Del(Map_t map_set, void* key) {
	RBT_node* result;
	WRLock_LockW(map_set->oplock);
	result = rbt_searchNodeByKey(map_set->map_store, key);
	if (result == NULL) {
		WRLock_UnlockW(map_set->oplock);
		return ERR_MAP_KEY_NOTFOUND;
	}
	--(map_set->cur_size);
	void* d_key = result->key;
	void* d_data = result->value;
	rbt_deleteNode(map_set->map_store , key);
	WRLock_UnlockW(map_set->oplock);
	map_set->key_free_func(d_key);
	map_set->val_free_func(d_data);
	return 0;
}
static void InorderTraversal(RBTree_t tree , RBT_node* x , foreach_call_back callback , void* args) {
	if (x != tree->nil) {
		InorderTraversal(tree , x->left , callback , args);
		callback(x->key , x->value , args);
		InorderTraversal(tree , x->right , callback, args);
	}
}
void Map_Foreach(Map_t map_set , foreach_call_back callback , void* args) {
	if (callback == NULL) {
		return;
	}
	WRLock_LockR(map_set->oplock);
	InorderTraversal(map_set->map_store , rbt_getRoot(map_set->map_store) , callback , args);
	WRLock_UnlockR(map_set->oplock);
}
typedef struct key_value {
	void* key;
	void* val;
}kv_pair;
static void _destroy_callback(const void* key, const void* data,void* args) {
	kv_pair new_pair;
	new_pair.key = (void*)key;
	new_pair.val = (void*)data;
	Queue_Push((Queue_t)args , &new_pair);
}
void Map_Destroy(Map_t map_set) {
	Queue_t node_queue = Queue_Init(sizeof(kv_pair));
	Map_Foreach(map_set , _destroy_callback , (void*)node_queue);
	rbt_destroyRBTree(map_set->map_store);
	kv_pair tmp;
	while(Queue_Pop(node_queue , &tmp) == QUEUE_OK) {
		map_set->key_free_func(tmp.key);
		map_set->val_free_func(tmp.val);
	}
	Queue_Destroy(node_queue);
	WRLock_Destroy(map_set->oplock);
	ylfree(map_set);
}














