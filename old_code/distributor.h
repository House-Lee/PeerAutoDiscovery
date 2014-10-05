/*
 * distributor.h
 *
 *  Created on: Jul 15, 2013
 *      Author: House_Lee
 */

#ifndef EXECUTOR_DISTRIBUTOR_H_
#define EXECUTOR_DISTRIBUTOR_H_

#include <stdint.h>
#include "commual.h"

#ifdef	__cplusplus
extern "C" {
#endif


static inline bool isValidRequest(char* msg) {
	if (!msg || *msg != 'v' || *(msg + 1) != '0' || *(msg + 2) != '0' || *(msg + 3) != '1') {
		return false;
	}
	int32_t msgid = *((int32_t*)msg + 1) ;
	if (msgid >= MSG_EXCU_END || msgid < 0) {
		return false;
	}
	int32_t msglen = *((int32_t*)msg + 1);
	if (msglen > MAX_PACKAGE_SIZE || msglen < 0) {
		return false;
	}
	return true;
}

void DistributeMsg(Message_t* msg);

#ifdef	__cplusplus
}
#endif

#endif /* EXECUTOR_DISTRIBUTOR_H_ */
