/*
 * msgtask.h
 *
 *  Created on: Jul 15, 2013
 *      Author: House_Lee
 */

#ifndef EXECUTOR_MSGTASK_H_
#define EXECUTOR_MSGTASK_H_

#include "commual.h"

#ifdef	__cplusplus
extern "C" {
#endif

int ReqTaskExecute(Client_t* cli, Message_t* msg);
int ReqTaskOutput(Client_t* cli, Message_t* msg);
int ReqTaskResult(Client_t* cli, Message_t* msg);

#ifdef	__cplusplus
}
#endif

#endif /* EXECUTOR_MSGTASK_H_ */
