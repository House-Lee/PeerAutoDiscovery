/*
 * network.h
 *
 *  Created on: Jul 9, 2013
 *      Author: House_Lee
 */

#ifndef INCLUDES_NETWORK_H_
#define INCLUDES_NETWORK_H_


#define NET_OK 0

typedef enum SOCKET_TYPE {
	_SOCK_TCP_ = 1,
	_SOCK_UDP_
}SOCKET_TYPE;

#ifdef	__cplusplus
extern "C" {
#endif


int	netCreateSocket(SOCKET_TYPE type);
int netSetNonBlock(char* err, int fd);
int netSetBlocking(char* err, int fd);

//TCP
int netTcpConnect(char* err, const char* addr , int port);
int netTcpNonBlockConnect(char* err, const char* addr , int port);
int netEnableTcpNoDelay(char* err, int fd);
int netDisableTcpNoDelay(char* err, int fd);
int netListenAndBind(char* err, int port);
int netTcpAccept(char* err, int listenfd , char* out_ip, int* out_port);

//UDP
int netUDPListen(char* err , int port);
int netUDPSendTo(char* err , int fd, char* destination , int port , char* send_buf , int ncount);
int netUDPRecv(char* err , int fd , char* recv_buf , int ncount);

//int netUnixAccept(int listenfd);//暂时用不到，先注释

//Operations Utilities
int netRead(char* err , int fd , char* recv_buf , int ncount);
int netReadNBytes(char* err, int fd , char* recv_buf , int ncount);
int netWrite(char* err, int fd , char* send_buf , int ncount);



#ifdef	__cplusplus
}
#endif

#endif /* INCLUDES_NETWORK_H_ */
