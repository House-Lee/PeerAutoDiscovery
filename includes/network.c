/*
 * network.c
 *
 *  Created on: Jul 9, 2013
 *      Author: House_Lee
 */

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
//#include <sys/un.h>//暂缓实现UNIX_DOMAIN SOCKET
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#include "commual.h"
#include "network.h"

enum TCP_CONNECT_FLAG {
	NETWORK_GENERAL_CONNECT = 0,
	NETWORK_NONBLOCK_CONNECT
};
static int tcp_connect(char* err, const char* addr , int port , int flags) {
	int sockfd;
	struct sockaddr_in sa;
	if ( (sockfd = netCreateSocket(_SOCK_TCP_)) == ERR_NETWORK_EXCEPTION) {
		if (err) {
			snprintf(err , MAX_NET_ERR_LEN , "create socket failed");
		}
		return ERR_NETWORK_EXCEPTION;
	}
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	if (inet_pton(AF_INET , addr , &sa) == 0) {
		//if addr is not in ipv4 format, try to parse it as hostname
		struct addrinfo hints;
		struct addrinfo* result;
		bzero(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		if (getaddrinfo(addr , (const char*)NULL , &hints , &result)) {
			close(sockfd);
			if (err) {
				snprintf(err, MAX_NET_ERR_LEN, "get socket ip address failed");
			}
			return ERR_NETWORK_EXCEPTION;
		}
		memcpy(&sa.sin_addr, &((struct sockaddr_in*)(result->ai_addr))->sin_addr , sizeof(sa.sin_addr));
	}
	if (flags & NETWORK_NONBLOCK_CONNECT) {
		if (netSetNonBlock(err, sockfd) == ERR_NETWORK_EXCEPTION) {
			close(sockfd);
			return ERR_NETWORK_EXCEPTION;
		}
	}
	//connect
	if (connect(sockfd , (struct sockaddr*)&sa ,sizeof(sa)) == -1) {
		if (errno == EINPROGRESS && (flags & NETWORK_NONBLOCK_CONNECT)) {
			return sockfd;
		}
		close(sockfd);
		if (err) {
			snprintf(err, MAX_NET_ERR_LEN, "connect failed");
		}
		return ERR_NETWORK_EXCEPTION;
	}
	return sockfd;
}

int	netCreateSocket(SOCKET_TYPE type) {
	int sock_fd;
	if (type == _SOCK_TCP_)
		sock_fd = socket(AF_INET , SOCK_STREAM , 0);
	else
		sock_fd = socket(AF_INET , SOCK_DGRAM , 0);
	if (sock_fd < 0)
		return ERR_NETWORK_EXCEPTION;
	return sock_fd;
}
int netSetNonBlock(char* err, int fd) {
	int flags;
	if ( (flags = fcntl(fd , F_GETFL , 0)) < 0) {
		if (err) {
			snprintf(err, MAX_NET_ERR_LEN, "get socket flag failed");
		}
		return ERR_NETWORK_EXCEPTION;
	}
	if (fcntl(fd , F_SETFL , flags | O_NONBLOCK) < 0) {
		if (err) {
			snprintf(err, MAX_NET_ERR_LEN, "set socket flag failed");
		}
		return ERR_NETWORK_EXCEPTION;
	}
	return 0;
}
int netSetBlocking(char* err, int fd) {
	int flags;
	if ((flags = fcntl(fd, F_GETFL, 0)) < 0) {
		if (err) {
			snprintf(err, MAX_NET_ERR_LEN, "get socket flag failed");
		}
		return ERR_NETWORK_EXCEPTION;
	}
	if (fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		if (err) {
			snprintf(err, MAX_NET_ERR_LEN, "set socket flag failed");
		}
		return ERR_NETWORK_EXCEPTION;
	}
	return 0;
}
int netTcpConnect(char* err, const char* addr , int port) {
	return tcp_connect(err, addr , port , NETWORK_GENERAL_CONNECT);
}
int netTcpNonBlockConnect(char* err, const char* addr , int port) {
	return tcp_connect(err, addr , port , NETWORK_NONBLOCK_CONNECT);
}

static int _set_tcp_nodelay(char* err, int fd , int type) {
	if (type != 0 && type != 1) {
		if (err) {
			snprintf(err, MAX_NET_ERR_LEN, "Unsupported type");
		}
		return ERR_NETWORK_EXCEPTION;
	}
	if (setsockopt(fd , IPPROTO_TCP , TCP_NODELAY , &type , sizeof(type)) == -1) {
		if (err) {
			snprintf(err, MAX_NET_ERR_LEN, "Set socket no-delay failed");
		}
		return ERR_NETWORK_EXCEPTION;
	}
	return NET_OK;
}
int netEnableTcpNoDelay(char* err, int fd) {
	return _set_tcp_nodelay(err, fd , 1);
}
int netDisableTcpNoDelay(char* err, int fd) {
	return _set_tcp_nodelay(err ,fd , 0);
}

int netListenAndBind(char* err, int port) {
	int sockfd;
	if ( (sockfd = netCreateSocket(_SOCK_TCP_)) == ERR_NETWORK_EXCEPTION) {
		if (err) {
			snprintf(err, MAX_NET_ERR_LEN, "socket create failed");
		}
		return ERR_NETWORK_EXCEPTION;
	}
	struct sockaddr_in serv_addr;
	bzero(&serv_addr , sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
	if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))) {
		if (err) {
			snprintf(err, MAX_NET_ERR_LEN, "bind failed");
		}
		return ERR_NETWORK_EXCEPTION;
	}
	if (listen(sockfd , SOMAXCONN)) {
		if (err) {
			snprintf(err, MAX_NET_ERR_LEN, "listen failed");
		}
		return ERR_NETWORK_EXCEPTION;
	}
	return sockfd;
}

static int net_accept(char* err, int listenfd , struct sockaddr* sa, socklen_t* len) {
	int fd;
	for(;;) {
		fd = accept(listenfd , sa , len);
		if (fd == -1) {
			if (errno == EINTR) {
				continue;
			} else if (errno == EAGAIN || errno == EWOULDBLOCK) {
				if (err) {
					err[0] = '\0';
				}
				return ERR_NETWORK_ACCEPT_ALLDONE;
			} else {
				if (err) {
					snprintf(err , MAX_NET_ERR_LEN , "%s" , strerror(errno));
				}
				return ERR_NETWORK_EXCEPTION;
			}
		}
		break;
	}
	return fd;
}
int netTcpAccept(char* err, int listenfd , char* out_ip, int* out_port) {
	int fd;
	struct sockaddr_in sa;
	socklen_t salen = sizeof(sa);
	if ((fd = net_accept(err, listenfd , (struct sockaddr*)&sa , &salen)) == ERR_NETWORK_EXCEPTION) {
		return ERR_NETWORK_EXCEPTION;
	}
	if (out_ip) {
		strcpy(out_ip , inet_ntoa(sa.sin_addr));
	}
	if (out_port) {
		*out_port = ntohs(sa.sin_port);
	}
	return fd;
}
//暂时用不到，先暂缓继续实现UNIX_DOMAIN SOCKET。 P.S.:本函数已完成
//int netUnixAccept(int listenfd) {
//	int fd;
//	struct sockaddr_un sa;
//	socklen_t salen = sizeof(sa);
//	if ((fd = net_accept(listenfd , (struct sockaddr*)&sa , &salen)) == ERR_NETWORK_EXCEPTION) {
//		return ERR_NETWORK_EXCEPTION;
//	}
//	return fd;
//}

int netRead(char* err, int fd , char* recv_buf , int ncount) {
	int recv_cnt = read(fd , recv_buf , ncount);
	if (recv_cnt < 0) {
		if (err) {
			snprintf(err , MAX_NET_ERR_LEN , "%s" , strerror(errno));
		}
		return ERR_NETWORK_EXCEPTION;
	}
	return recv_cnt;
}

int netReadNBytes(char* err,int fd , char* recv_buf , int ncount) {
	int recv_cnt = 0, nread;
	while(recv_cnt < ncount) {
		nread = read(fd , recv_buf + recv_cnt , ncount - recv_cnt);
		if (nread == 0) {
			return recv_cnt;
		}
		if (nread < 0) {
			if (err) {
				snprintf(err , MAX_NET_ERR_LEN , "%s" , strerror(errno));
			}
			return ERR_NETWORK_EXCEPTION;
		}
		recv_cnt += nread;
	}
	return recv_cnt;
}
int netWrite(char* err, int fd , char* send_buf , int ncount) {
	int send_cnt  = 0, nsend;
	while(send_cnt < ncount) {
		nsend = write(fd , send_buf + send_cnt , ncount - send_cnt);
		if (nsend == 0) {
			return send_cnt;
		}
		if (nsend < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				return ERR_NETWORK_TRY_AGAIN;
			}
			if (err) {
				snprintf(err, MAX_NET_ERR_LEN, "%s", strerror(errno));
			}
			return ERR_NETWORK_EXCEPTION;
		}
		send_cnt += nsend;
	}
	return send_cnt;
}









