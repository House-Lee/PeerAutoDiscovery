/*
 * distributor.c
 *
 *  Created on: Jul 15, 2013
 *      Author: House_Lee
 */

#include "distributor.h"
#include "log.h"
#include "executor.h"
#include "publicreq.h"
#include "msgtask.h"

typedef int (*msg_handle_func)(Client_t* cli, Message_t* msg);

static msg_handle_func HandlerMap[MSG_EXCU_END] = {
		ClientPluse,
		ClientAuth,
		ReqTaskExecute,
		ReqTaskOutput,
		ReqTaskResult,
		ConfigGet,
		ConfigSet,
};

void DistributeMsg(Message_t* msg) {
	if (!msg || msg->msgid < 0 || msg->msgid >= MSG_EXCU_END) {
		//Since this has been already checked in _read_query_from_client(),
		//this should never happened.
		Log(LOG_ERROR,"Msg not recognized ");
		return;
	}
	Client_t* c = Map_GetPtr(g_executor.clients , &msg->sockfd);
	if (c == NULL) {
		Log(LOG_NOTICE,"client has gone. Request:[%d] ignored", msg->msgid);
		return;
	}
	StdMutex_Lock(c->oplock);
	int res = (*HandlerMap[msg->msgid])(c , msg);
	if (res) {
		Log(LOG_NOTICE,"Messge[%d] from[%s:%d] process failed.CODE[%d]", msg->msgid , c->addr , c->port , res);
	}
	StdMutex_Unlock(c->oplock);
}
