/*
 * publicreq.c
 *
 *  Created on: Jul 16, 2013
 *      Author: House_Lee
 */

#include <time.h>
#include "commual.h"
#include "publicreq.h"
#include "log.h"
#include "executor.h"
#include "../protos/compiled/common_msg.pb-c.h"

int ClientPluse(Client_t* cli, Message_t* msg) {
	int rtn = 0;
	Pluse* request;
	ReqUnpack(request , pluse , msg);
	Log(LOG_DEBUG,"ClientPluse():Recv Client [%s:%d] pluse:%d" , cli->addr , cli->port , request->timestamp);
	ReqFree(request , pluse);
	time_t now_time = time(NULL);
	cli->last_seen = now_time;
	Pluse AckReturn = PLUSE__INIT;
	AckReturn.timestamp = (int32_t)now_time;
	SendAck(cli->fd , MSG_EXCU_PLUSE , pluse , AckReturn);
	return rtn;
}
int ClientAuth(Client_t* cli, Message_t* msg) {
	int rtn = 0;
	ReqAuth* request;
	ReqUnpack(request , req_auth , msg);
	Log(LOG_DEBUG,"ClientAuth():New Client Auth.[%d%s%s]" , request->timestamp , request->auth_key , request->sign);
	ReqFree(request , req_auth);
	cli->status = CLI_ONLINE;
	AckCommon AckReturn = ACK_COMMON__INIT;
	AckReturn.code = 0;
	AckReturn.msg = "ok";
	SendAck(cli->fd , MSG_EXCU_AUTH , ack_common , AckReturn);
	return rtn;
}

int ConfigGet(Client_t* cli , Message_t* msg) {
	int rtn = 0;
	ReqGetConfig* request;
	ReqUnpack(request , req_get_config , msg);
	char* str_config = ExecutorGetConfig(request->key);
	AckCommon AckReturn = ACK_COMMON__INIT;
	if (NULL == str_config) {
		Log(LOG_DEBUG , "ConfigGet()[%s] unrecognized" , request->key);
		rtn = ERR_EXECUTOR_CONFIG_UNRECOGNIZED;
		AckReturn.msg = "Configure unrecognized";
	} else {
		Log(LOG_DEBUG , "ConfigGet()[%s]->[%s]" , request->key , str_config);
		AckReturn.msg = str_config;
	}
	ReqFree(request , req_get_config);
	AckReturn.code = rtn;
	SendAck(cli->fd , MSG_EXCU_CONF_GET , ack_common , AckReturn);
	ExecutorFreeResult(str_config);
	return rtn;
}

int ConfigSet(Client_t* cli , Message_t* msg) {
	int rtn = 0;
	ReqSetConfig* request;
	ReqUnpack(request , req_set_config , msg);
	rtn = ExecutorSetConfig(request->key , request->value);
	Log(LOG_DEBUG , "ConfigSet():[%s:%d] requst to set [%s]->[%s]. retcode:%d" , cli->addr ,cli->port , request->key , request->value , rtn);
	ReqFree(request , req_set_config);
	AckCommon AckReturn = ACK_COMMON__INIT;
	AckReturn.code = rtn;
	SendAck(cli->fd , MSG_EXCU_CONF_SET , ack_common , AckReturn);
	return rtn;
}
