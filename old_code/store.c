/*
 * store.c
 *
 *  Created on: Jul 19, 2013
 *      Author: House_Lee
 */

#include <string.h>
#include <sys/types.h>
#include "store.h"
#include "commual.h"
#include "../includes/memory.h"
#include "log.h"

redisContext* redis_connection;
static char* storage_server;
static int	 storage_port;

#define ST_OK 0
#define ST_ERR 1
static int _connect_to_storage() {
	if (redis_connection) {
		redisFree(redis_connection);
	}
	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	redis_connection = redisConnectWithTimeout(storage_server , storage_port , timeout);
	if (redis_connection == NULL || redis_connection->err) {
		Log(LOG_ERROR,"[STORAGE ERROR] Fail to connect to storage server");
		return ST_ERR;
	}
	redisSetTimeout(redis_connection , timeout);
	return ST_OK;
}
int StoreInit(const char* serv , int port) {
	storage_server = ylstrdup(serv);
	storage_port = port;
	redis_connection = NULL;
	if (_connect_to_storage() != ST_OK) {
		return ERR_STORAGE_INIT_FAIL;
	}
	return 0;
}

int StoreSet(const char* key, const char* value) {
	redisReply* reply = redisCommand(redis_connection , "SET YL_EXECUTOR_%s %s" , key, value);
	if (reply == NULL) {
		_connect_to_storage();
		reply = redisCommand(redis_connection , "SET YL_EXECUTOR_%s %s" , key, value);
		if (reply == NULL) {
			return ERR_STORAGE_SET_FAIL;
		}
	}
	if (reply)
		freeReplyObject(reply);
	return 0;
}

int StoreAppend(const char* key, const char* value) {
	redisReply* reply = redisCommand(redis_connection, "APPEND YL_EXECUTOR_%s %s", key, value);
	if (reply == NULL) {
		_connect_to_storage();
		reply = redisCommand(redis_connection, "APPEND YL_EXECUTOR_%s %s", key, value);
		if (reply == NULL) {
			return ERR_STORAGE_APPEND_FAIL;
		}
	}
	freeReplyObject(reply);
	return 0;
}

int StoreSetBinary(const char* key, void* src, int len) {
	redisReply* reply = redisCommand(redis_connection , "SET YL_EXECUTOR_%s %b" , key, src , (size_t)len);
	if (reply == NULL) {
		_connect_to_storage();
		reply = redisCommand(redis_connection , "SET YL_EXECUTOR_%s %b" , key, src , (size_t)len);
		if (reply == NULL) {
			return ERR_STORAGE_SET_FAIL;
		}
	}
	if (reply)
		freeReplyObject(reply);
	return 0;
}
int StoreGetBinary(const char* key, void** out, int* outlen) {
	if (!out || !outlen) {
		return ERR_STORAGE_WRONG_PARAM;
	}
	*out = NULL;
	*outlen = 0;
	redisReply* reply = redisCommand(redis_connection, "GET YL_EXECUTOR_%s" , key);
	if (reply == NULL) {
		_connect_to_storage();
		reply = redisCommand(redis_connection, "GET YL_EXECUTOR_%s" , key);
		if (reply == NULL) {
			return ERR_STORAGE_GETBINARY_FAIL;
		}
	}
	int ret = 0;
	char* rtn = NULL;
	if (reply->type == REDIS_REPLY_STRING) {
		rtn = ylmalloc(reply->len + 1);
		memcpy(rtn , reply->str , reply->len);
		rtn[reply->len] = '\0';
		*out = rtn;
		*outlen = reply->len;
	} else {
		ret = ERR_STORAGE_GETBINARY_FAIL;
	}
	freeReplyObject(reply);
	return ret;
}
char* StoreGet(const char* key) {
	redisReply* reply = redisCommand(redis_connection, "GET YL_EXECUTOR_%s" , key);
	if (reply == NULL) {
		_connect_to_storage();
		reply = redisCommand(redis_connection, "GET YL_EXECUTOR_%s" , key);
		if (reply == NULL) {
			return NULL;
		}
	}
	char* rtn = NULL;
	if (reply->type == REDIS_REPLY_STRING) {
		rtn = ylmalloc(reply->len + 1);
		memcpy(rtn , reply->str , reply->len);
		rtn[reply->len] = '\0';
	}
	freeReplyObject(reply);
	return rtn;
}
void StoreFree(char* value) {
	if (value) {
		ylfree(value);
	}
}

















