/*
 * publicreq.h
 *
 *  Created on: Jul 16, 2013
 *      Author: House_Lee
 */

#ifndef PUBLICREQ_H_
#define PUBLICREQ_H_


#ifdef	__cplusplus
extern "C" {
#endif

int ClientPluse(Client_t* cli, Message_t* msg);
int ClientAuth(Client_t* cli, Message_t* msg);


int ConfigGet(Client_t* cli , Message_t* msg);
int ConfigSet(Client_t* cli , Message_t* msg);

#ifdef	__cplusplus
}
#endif

#endif /* PUBLICREQ_H_ */
