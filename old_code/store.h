/*
 * store.h
 *
 *  Created on: Jul 19, 2013
 *      Author: House_Lee
 */

#ifndef STORE_H_
#define STORE_H_


#ifdef	__cplusplus
extern "C" {
#endif

#include <hiredis/hiredis.h>

extern redisContext* redis_connection;

int StoreInit(const char* serv , int port);
int StoreSet(const char* key, const char* value);
int StoreSetBinary(const char* key, void* src, int len);
int StoreAppend(const char* key, const char* value);
int StoreGetBinary(const char* key, void** out, int* outlen);
char* StoreGet(const char* key);
void StoreFree(char* value);


#ifdef	__cplusplus
}
#endif

#endif /* STORE_H_ */
