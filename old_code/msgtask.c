/*
 * msgtask.c
 *
 *  Created on: Jul 15, 2013
 *      Author: House_Lee
 */

#include <string.h>
#include "msgtask.h"
#include "task.h"
#include "log.h"
#include "../protos/compiled/common_msg.pb-c.h"
#include "../protos/compiled/taskmsg.pb-c.h"

int ReqTaskExecute(Client_t* cli, Message_t* msg) {
	int rtn = 0;
	ReqExecuteTask* request;
	ReqUnpack(request , req_execute_task , msg);
	rtn = Task_Create(request->task_id , request->command);
	Log(LOG_DEBUG,"ReqTaskExecute()client[%s:%d]request to execute [%s]%s" , cli->addr , cli->port , request->task_id , request->command);
	ReqFree(request , req_execute_task);
	AckCommon ackreturn = ACK_COMMON__INIT;
	ackreturn.code = rtn;
	if (!rtn) {
		ackreturn.msg = "ok";
	}
	SendAck(cli->fd , MSG_EXCU_EXETASK , ack_common , ackreturn);
	return rtn;
}
int ReqTaskOutput(Client_t* cli, Message_t* msg) {
	int rtn = 0;
	ReqTaskCommon* request;
	ReqUnpack(request , req_task_common , msg);
	char* output = Task_GetOutput(request->task_id);
//	ReqFree(request , req_task_common);
	AckTaskOutput ackreturn = ACK_TASK_OUTPUT__INIT;
	if (output == NULL) {
		rtn = MSG_ERR_TASK_NOT_FOUND;
		ackreturn.req_result = MSG_ERR_TASK_NOT_FOUND;
		ackreturn.output = "TASK NOT FOUND";
	} else {
		ackreturn.req_result = 0;
		ackreturn.output = output;
	}
	Log(LOG_DEBUG,"ReqTaskOutput()client[%s:%d]request[%s].reply length:[%zu][%.20s...]" ,cli->addr , cli->port, request->task_id , strlen(ackreturn.output) , ackreturn.output);
	ReqFree(request , req_task_common);
	SendAck(cli->fd , MSG_EXCU_GETOUTPUT , ack_task_output , ackreturn);
	Task_FreeResult(output);
	return rtn;
}
int ReqTaskResult(Client_t* cli, Message_t* msg) {
	int rtn = 0;
	ReqTaskCommon* request;
	ReqUnpack(request, req_task_common, msg);
	char* str_result;
	int	  int_result;
	rtn = Task_GetResult(request->task_id , &int_result , &str_result);
	Log(LOG_DEBUG,"ReqTaskResult():client[%s:%d] request the result of [%s]." , cli->addr , cli->port , request->task_id);
	ReqFree(request , req_task_common);
	AckTaskResult ackreturn = ACK_TASK_RESULT__INIT;
	ackreturn.req_result = rtn;
	if (str_result) {
		ackreturn.task_str_result = str_result;
	} else {
		ackreturn.task_str_result = "get result error";
	}
	ackreturn.task_result = int_result;
	Log(LOG_DEBUG,"ReqTaskResult():Result Send to client[%s:%d]. [%d]%.20s" , cli->addr , cli->port  , int_result ,ackreturn.task_str_result );
	SendAck(cli->fd , MSG_EXCU_GETRES , ack_task_result , ackreturn);
	Task_FreeResult(str_result);
	return rtn;
}














