/*
 * commual.h
 *
 *  Created on: Jul 8, 2013
 *      Author: House_Lee
 */

#ifndef EXECUTOR_COMMUAL_H_
#define EXECUTOR_COMMUAL_H_

#include <stdint.h>
#include <time.h>
#include <google/protobuf-c/protobuf-c.h>
#include "../includes/commual.h"
#include "../includes/lock.h"

#define DEFAULT_EXECUTOR_RUN_PORT 		7016
#define	DEFAULT_EXECUTOR_PLUSE_INTVAL	15
#define DEFAULT_EXECUTOR_DAEMONIZE		false
#define DEFAULT_EXECUTOR_LOG_PATH 		"./"
#define DEFAULT_EXECUTOR_LOG_FILE 		"executor.log"
#define DEFAULT_EXECUTOR_STORAGE_SERVER	"127.0.0.1:17016"
#define DEFAULT_EXECUTOR_CONCURRENT		50
#define DEFAULT_EXECUTOR_SAVE_RESULT_TO "http://yunlu.qiyi.virtual/dev/?r=missionctl/saveresult"

#define MAX_CONCURRENT 256
#define MAX_CLIENTS 2048
#define CLIENT_RECV_QUEUES 2
#define CLIENT_SEND_QUEUES 2
#define CLIENTS_BUFFER_LEN 1024*20
#define MAX_PACKAGE_SIZE 1024*10

#define MSGHEAD_SIZE	20

#ifdef	__cplusplus
extern "C" {
#endif


enum EXECUTOR_MSGID {
	MSG_EXCU_PLUSE = 0,
	MSG_EXCU_AUTH = 1,
	MSG_EXCU_EXETASK = 2,
	MSG_EXCU_GETOUTPUT = 3,
	MSG_EXCU_GETRES = 4,
	MSG_EXCU_CONF_GET = 5,
	MSG_EXCU_CONF_SET = 6,
	MSG_EXCU_END,
};

enum LOG_LEVEL {
	LOG_LEV_START = 0,
	LOG_FORCE,
	LOG_ERROR,
	LOG_WARNING,
	LOG_NOTICE,
	LOG_DEBUG,
	LOG_LEV_END,
};

typedef struct {
	int32_t 	sockfd;
	int32_t		msgid;
	int32_t		pkg_len;
	uint8_t*	package;
	int32_t		trysend_cnt;
}Message_t;

enum CLIENT_STATUS {
	CLI_INIT = 0,
	CLI_ONLINE,
};

typedef struct {
	int32_t		fd;
	char		addr[IP_ADDR_LEN];
	int			port;
	time_t		last_seen;//Pluse
	int			status;
	int			buffer_len;
	char		query_buffer[CLIENTS_BUFFER_LEN];
	StdMutex_t	oplock;
}Client_t;


extern ProtobufCAllocator g_pb_allocator;
#define ReqUnpack(reqInfo, pbname , pMsg) {\
	reqInfo = pbname##__unpack(&g_pb_allocator , pMsg->pkg_len , pMsg->package);\
	if (!reqInfo) \
		return -1001;\
}
#define ReqFree(reqInfo , pbname) {\
	pbname##__free_unpacked(reqInfo , &g_pb_allocator);\
}

#define SendAck(fd , msgid, pbname, pbvalue) {\
		uint8_t *buf;\
		int nLen = pbname##__get_packed_size(&pbvalue) + MSGHEAD_SIZE;\
		buf = ylmalloc(nLen);\
		pbname##__pack(&pbvalue , buf + MSGHEAD_SIZE);\
		ReplyMessage(fd,msgid , nLen , buf);\
}
void ReplyMessage(int fd , int msgid , int pkg_len, uint8_t* package);

//NOTE: when replying an message, pack head and package into msg.package

#ifdef	__cplusplus
}
#endif

#endif /* EXECUTOR_COMMUAL_H_ */
