/*
 * executor.c
 *
 *  Created on: Jul 8, 2013
 *      Author: House_Lee
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include "executor.h"
#include "task.h"
#include "../includes/memory.h"
#include "../includes/util.h"
#include "../includes/network.h"
#include "../includes/dstr.h"
#include "log.h"
#include "distributor.h"

Executor_t g_executor;
ProtobufCAllocator g_pb_allocator;


static void _sig_term_handler_(int sig) {
	AVOID_NOUSED(sig);
	int fd = g_executor.log_path_ ? open(g_executor.log_path_ , O_APPEND|O_CREAT|O_WRONLY, 0644) : STDOUT_FILENO;
	if (fd == -1) {
		Event_Stop(g_executor.evloop);
	}
	write(fd , "SERVER SHUTDOWN!\n" , 17);
	close(fd);
	Event_Stop(g_executor.evloop);
	sleep(3);//wait 3 seconds for processing unsaved events
	exit(1);//force exit if there is still not any events incomming after 3 seconds
}

static void* _executor_sig_monitor_thread(void* arg) {
	sigset_t* towait = (sigset_t*)arg;
	int sig;
	for(;;) {
		if (sigwait(towait, &sig)) {
			Log(LOG_ERROR, "sigwait error");
			continue;
		}
		if(sig == SIGTERM) {
			_sig_term_handler_(sig);
		} else if (sig == SIGCHLD) {
			_sig_chld_handler_(sig);
		}
		continue;
	}
	return NULL;
}
extern int pthread_sigmask(int, const sigset_t *, sigset_t *);
int ExcutorInitSingalHandlers() {
	signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	sigset_t* sig = ylmalloc(sizeof(sigset_t));
	sigemptyset(sig);
	sigaddset(sig , SIGTERM);
	sigaddset(sig , SIGCHLD);
	if (pthread_sigmask(SIG_BLOCK, sig, NULL)) {
		return ERR_TASK_MONITOR_INIT_FAIL;
	}
	pthread_t sig_thread;
	pthread_create(&sig_thread, NULL, _executor_sig_monitor_thread, (void*) sig);
	return 0;
}

int ExecutorLoadConfig(const char* filename) {
	g_executor.port_ = DEFAULT_EXECUTOR_RUN_PORT;
	g_executor.pluse_interval_ = DEFAULT_EXECUTOR_PLUSE_INTVAL;
	g_executor.max_concurrent_task_ = DEFAULT_EXECUTOR_CONCURRENT;
	g_executor.log_level = LOG_DEBUG;
	int tmp_log_path_len = strlen(DEFAULT_EXECUTOR_LOG_PATH) + strlen(DEFAULT_EXECUTOR_LOG_FILE) + 1;
	g_executor.log_path_ = ylmalloc(tmp_log_path_len);
	snprintf(g_executor.log_path_, tmp_log_path_len,
			"%s%s", DEFAULT_EXECUTOR_LOG_PATH, DEFAULT_EXECUTOR_LOG_FILE);
	char* tmp_server = ylstrdup(DEFAULT_EXECUTOR_STORAGE_SERVER);
	char* str_pos = strstr(tmp_server, ":");
	if (!str_pos || !is_numeric(str_pos + 1)) {
		/*this should not be happened, unless someone had written some incorrect code*/
		ylfree(g_executor.log_path_);
		ylfree(tmp_server);
		return ERR_EXECUTOR_CONFIG_INCORRECT;
	}
	*str_pos++ = '\0';
	snprintf(g_executor.store_server_addr_, IP_ADDR_LEN, "%s", tmp_server);
	g_executor.store_server_port_ = atoi(str_pos);
	ylfree(tmp_server);
	g_executor.save_result_to = ylstrdup(DEFAULT_EXECUTOR_SAVE_RESULT_TO);

	//Read Configure File
	FILE* fp;
	if (!filename) {
		return 0;
	}
	if ((fp = fopen(filename , "r")) == NULL) {
		return ERR_EXECUTOR_CONFIG_FILE_NOT_EXIST;
	}
	char line_buffer[MAX_CONFIG_LINE];
	char filter_line[MAX_CONFIG_LINE];
	int c;
	int pos = 0;
	while( (c = fgetc(fp)) != EOF ) {
		if ('\n' == c || pos == MAX_CONFIG_LINE_LEN) {
			line_buffer[pos] = '\0';
			//cancel the notation
			str_pos = strstr(line_buffer , "#");
			if (str_pos) {
				*str_pos = '\0';
			}
			//trim the input buffer
			pos = 0;
			str_pos = line_buffer;
			while(*str_pos != '\0') {
				if (*str_pos != ' ' && *str_pos != '\t') {
					filter_line[pos++] = *str_pos;
				}
				++str_pos;
			}
			filter_line[pos] = '\0';
			if (pos) {
//				printf("[%s]\n",filter_line);
				str_pos = strstr(filter_line , "=");
				if (!str_pos || !*(str_pos + 1)) {
					return ERR_EXECUTOR_CONFIG_INCORRECT;
				}
				*str_pos++ = '\0';
				if (!strncmp("port" , filter_line , pos)) {
					if (!is_numeric(str_pos)) {
						return ERR_EXECUTOR_CONFIG_INCORRECT;
					}
					g_executor.port_ = atoi(str_pos);
				} else if (!strncmp("concurrent_task" , filter_line , pos)) {
					if (!is_numeric(str_pos)) {
						return ERR_EXECUTOR_CONFIG_INCORRECT;
					}
					g_executor.max_concurrent_task_ = atoi(str_pos);
				} else if (!strncmp("loglevel" , filter_line , pos)) {
					int loglevel;
					if (!strncmp("ERROR", str_pos, 5)) {
						loglevel = LOG_ERROR;
					} else if (!strncmp("WARNING", str_pos, 7)) {
						loglevel = LOG_WARNING;
					} else if (!strncmp("NOTICE", str_pos, 6)) {
						loglevel = LOG_NOTICE;
					} else if (!strncmp("DEBUG", str_pos, 5)) {
						loglevel = LOG_DEBUG;
					} else {
						return ERR_EXECUTOR_CONFIG_INCORRECT;
					}
					g_executor.log_level = loglevel;
				} else if (!strncmp("pluse_interval" , filter_line , pos)) {
					if (!is_numeric(str_pos)) {
						return ERR_EXECUTOR_CONFIG_INCORRECT;
					}
					g_executor.pluse_interval_ = atoi(str_pos);
					if (g_executor.pluse_interval_ < DEFAULT_EXECUTOR_PLUSE_INTVAL) {
						g_executor.pluse_interval_ = DEFAULT_EXECUTOR_PLUSE_INTVAL;
					}
				} else if (!strncmp("log_path" , filter_line , pos)) {
					int tmp_len = pos - (int)(str_pos - filter_line);
					if (tmp_len >= strlen(g_executor.log_path_)) {
						g_executor.log_path_ = ylrealloc(g_executor.log_path_ , tmp_len + 1);
					}
					snprintf(g_executor.log_path_ , tmp_len + 1, "%s" , str_pos);
				} else if (!strncmp("storage_server" , filter_line , pos)) {
					char* tmp = strstr(str_pos , ":");
					if (!tmp || !is_numeric(tmp + 1)) {
						return ERR_EXECUTOR_CONFIG_INCORRECT;
					}
					*tmp++ = '\0';
					snprintf(g_executor.store_server_addr_ , IP_ADDR_LEN , "%s" , str_pos);
					g_executor.store_server_port_ = atoi(tmp);
				} else if (!strncmp("save_result_to" , filter_line , pos)) {
					int tmp_len = pos - (int)(str_pos - filter_line);
					if (tmp_len >= strlen(g_executor.save_result_to)) {
						g_executor.save_result_to = ylrealloc(g_executor.save_result_to , tmp_len + 1);
					}
					snprintf(g_executor.save_result_to , tmp_len + 1, "%s" , str_pos);
 				} else if (!strncmp("daemonize" , filter_line , pos)) {
					if (!strncmp(str_pos , "yes" , 3)) {
						g_executor.daemonize = true;
					} else {
						g_executor.daemonize = false;
					}
				} else {
					printf("%s\n" , filter_line);
					return ERR_EXECUTOR_CONFIG_UNRECOGNIZED;
				}
			}
			pos = 0;
		} else {
			line_buffer[pos++] = (char)c;
		}
	}
	return 0;
}

int ExecutorSetConfig(const char* key, const char* value) {
	if (!strncmp("loglevel" , key , 8)) {
		int loglevel = 0;
		if (!strncmp("ERROR" , value , 5)) {
			loglevel = LOG_ERROR;
		} else if (!strncmp("WARNING" , value , 7)) {
			loglevel = LOG_WARNING;
		} else if (!strncmp("NOTICE" , value , 6)) {
			loglevel = LOG_NOTICE;
		} else if (!strncmp("DEBUG" , value , 5)) {
			loglevel = LOG_DEBUG;
		} else {
			return ERR_EXECUTOR_CONFIG_INCORRECT;
		}
		WRLock_LockW(g_executor.conf_lock_);
		g_executor.log_level = loglevel;
		WRLock_UnlockW(g_executor.conf_lock_);
		Log_SetLevel(loglevel);
	} else if (!strncmp("pluse_interval" , key , 14)) {
		if (!is_numeric(value)) {
			return ERR_EXECUTOR_CONFIG_INCORRECT;
		}
		int pluse_interval = atoi(value);
		if (pluse_interval < DEFAULT_EXECUTOR_PLUSE_INTVAL) {
			pluse_interval = DEFAULT_EXECUTOR_PLUSE_INTVAL;
		}
		WRLock_LockW(g_executor.conf_lock_);
		g_executor.pluse_interval_ = pluse_interval;
		WRLock_UnlockW(g_executor.conf_lock_);
	} else {
		return ERR_EXECUTOR_CONFIG_SET_UNSUPPORTED;
	}
	return 0;
}
char* ExecutorGetConfig(const char* key) {
	char* rtn = NULL;
	if (!strncmp("loglevel" , key , 8)) {
		WRLock_LockR(g_executor.conf_lock_);
		rtn = ylstrdup(log_level_map[g_executor.log_level]);
		WRLock_UnlockR(g_executor.conf_lock_);
	} else if (!strncmp("port" , key , 4)) {
		rtn = ylmalloc(10);
		snprintf(rtn , 10 , "%d" , g_executor.port_);
	} else if (!strncmp("concurrent_task" , key , 15)) {
		rtn = ylmalloc(5);
		snprintf(rtn , 5 , "%d" , g_executor.max_concurrent_task_);
	} else if (!strncmp("pluse_interval" , key , 14)) {
		rtn = ylmalloc(10);
		WRLock_LockR(g_executor.conf_lock_);
		snprintf(rtn , 10 , "%d" , g_executor.pluse_interval_);
		WRLock_UnlockR(g_executor.conf_lock_);
	} else if (!strncmp("log_path" , key , 8)) {
		rtn = ylstrdup(g_executor.log_path_);
	} else if (!strncmp("storage_server" , key , 14)) {
		rtn = ylmalloc(256);
		snprintf(rtn , 256, "%s:%d",g_executor.store_server_addr_ , g_executor.store_server_port_);
	} else if (!strncmp("save_result_to" , key , 14)) {
		rtn = ylstrdup(g_executor.save_result_to);
	}
	return rtn;
}
void ExecutorFreeResult(char* ptr) {
	ylfree(ptr);
}

static void* _client_dup_func(const void* src_client) {
	Client_t* client = ylmalloc(sizeof(Client_t));
	memcpy(client , src_client , sizeof(Client_t));
	return client;
}
#define RELEASE_BY_CLIENT 0
#define RELEASE_BY_PLUSE 1
#define RELEASE_BY_ERROR 2
static void _release_client(int fd , int type) {
	static char* type_map[] = {
			"By Client",
			"By Pluse",
			"By Error"
	};
	if (type > 2 || type < 0) {
		type = RELEASE_BY_ERROR;
	}
	Client_t* stored_client = Map_GetAndDel(g_executor.clients , &fd);
	if (stored_client == NULL) {
		return;
	}
	StdMutex_Lock(stored_client->oplock);
	Log(LOG_NOTICE,"[RELEASE_CLIENT][%s:%d] type:[%s]" , stored_client->addr , stored_client->port , type_map[type]);
	Event_DeleteFileEvent(g_executor.evloop , stored_client->fd , EV_READABLE);
	close(stored_client->fd);
	StdMutex_Unlock(stored_client->oplock);
	StdMutex_Destroy(stored_client->oplock);
	ylfree(stored_client);
}

static void _read_query_from_client(EventLoop_t evloop , int fd , void* clientData , int mask) {
	AVOID_NOUSED(evloop);
	AVOID_NOUSED(mask);
//	AVOID_NOUSED(clientData);
//	Client_t* c = Map_GetPtr(g_executor.clients , &fd);
	Client_t* c = (Client_t*)clientData;
	int nread;
	char errmsg[MAX_NET_ERR_LEN];
	StdMutex_Lock(c->oplock);
	nread = netRead(errmsg, c->fd, c->query_buffer + c->buffer_len, CLIENTS_BUFFER_LEN - c->buffer_len);
	if (nread == ERR_NETWORK_EXCEPTION) {
		//ERR HAPPEND
		StdMutex_Unlock(c->oplock);
		Log(LOG_ERROR,"Read query from client failed.reason:##%s##", errmsg);
		_release_client(fd, RELEASE_BY_ERROR);
		return;
	} else if (nread == 0) {
		//Client Close the connection
		Log(LOG_NOTICE,"[CLIENT_DISCONNECT][%s:%d]", c->addr, c->port);
		StdMutex_Unlock(c->oplock);
		_release_client(fd, RELEASE_BY_CLIENT);
		return;
	}
	c->buffer_len += nread;
	//try to extract as many package as possible
	char* iterator = c->query_buffer;
	while (1) {
		if (c->buffer_len < MSGHEAD_SIZE) {
			break;
		}
		if (!isValidRequest(iterator)) {
			//not a valid request, clear client
			Log(LOG_NOTICE,"invalid request received from [%s:%d], close the connection" , c->addr , c->port);
			StdMutex_Unlock(c->oplock);
			_release_client(fd , RELEASE_BY_ERROR);
			return;
		}
		Message_t new_msg;
		new_msg.msgid = *((int32_t*)iterator + 1);
		new_msg.pkg_len = *((int32_t*)iterator + 2);
		if (c->status == CLI_INIT && new_msg.msgid != MSG_EXCU_AUTH && new_msg.msgid != MSG_EXCU_PLUSE) {
			//client not login, kick out
			Log(LOG_NOTICE,"client [%s:%d] should get authentication first before anyother request" , c->addr , c->port);
			StdMutex_Unlock(c->oplock);
			_release_client(fd , RELEASE_BY_ERROR);
			return;
		}
		c->last_seen = time(NULL);
		if (new_msg.pkg_len + MSGHEAD_SIZE > c->buffer_len) {
			//not a complete package
			int offset = iterator - c->query_buffer;
			for (int pos = 0; pos != c->buffer_len; ++pos) {
				c->query_buffer[pos] = c->query_buffer[pos + offset];
			}
			break;
		}
		//extract one package
		new_msg.sockfd = c->fd;
		uint8_t* pkg = ylmalloc(new_msg.pkg_len);
		memcpy(pkg , iterator + MSGHEAD_SIZE , new_msg.pkg_len);
		new_msg.package = pkg;
		uint8_t recv_buf = atomic_add_then_fetch((int*)&g_executor.cur_recv_buffer_ , 1)%CLIENT_RECV_QUEUES;
		Queue_Push(g_executor.recv_buffer_[recv_buf] , &new_msg);
		sem_post(&g_executor.recv_buffer_num_[recv_buf]);
		int msglen = new_msg.pkg_len + MSGHEAD_SIZE;
		c->buffer_len -= msglen;
		iterator += msglen;
	}
	StdMutex_Unlock(c->oplock);
}

static void _create_new_client(int fd, const char* ip, int port) {
	if (fd <= 0)
		return;
	netSetNonBlock(NULL, fd);
	netEnableTcpNoDelay(NULL, fd);
	Client_t client;
	client.fd = fd;
	snprintf(client.addr , IP_ADDR_LEN , "%s" , ip);
	client.port = port;
	client.last_seen = time(NULL);
	client.status = CLI_INIT;
	client.buffer_len = 0;
	client.oplock = StdMutex_Init();

	Client_t* stored_client = Map_SetAndGetPtr(g_executor.clients , &fd , &client);
	if (Event_CreateFileEvent(g_executor.evloop , fd , EV_READABLE , _read_query_from_client , stored_client) != EV_OK) {
		Log(LOG_ERROR,"Create new client failed.[%s:%d]" , stored_client->addr ,stored_client->port);
		close(fd);
		Map_Del(g_executor.clients , &fd);
		return;
	}
}

static void _request_accept_func( EventLoop_t evloop , int fd , void* clientData , int mask) {
	AVOID_NOUSED(evloop);
	AVOID_NOUSED(mask);
	AVOID_NOUSED(clientData);

	char client_ip[IP_ADDR_LEN];
	int	client_port,client_fd;
	char errmsg[MAX_NET_ERR_LEN];
	while(1) {
		client_fd = netTcpAccept(errmsg, g_executor.core_fd , client_ip , &client_port);
		if (client_fd == ERR_NETWORK_ACCEPT_ALLDONE) {
			break;
		} else if (client_fd == ERR_NETWORK_EXCEPTION){
			Log(LOG_ERROR,"ACCEPTING NEW CONNECTION FAILED. reason:##%s##",errmsg);
			break;
		}
		Log(LOG_NOTICE,"[NEW CLIENT] -- %s:%d --" , client_ip , client_port);
		_create_new_client(client_fd , client_ip , client_port);
	}
}


static void _executor_start_printscreen(void) {
	char* screen =
		"###################################################################################\n"
		" # ============================================================================= #\n"
		" # Yunlu Server Remote Management - Task Executor                                #\n"
		" # Copyright (c) 2013 iQIYI inc.                                                 #\n"
		" #                                                                               #\n"
		" # Author: House.Lee (lixiaodan@qiyi.com)                                        #\n"
		" # Created at: 2013-07-09 14:10                                                  #\n"
		" #                                                                               #\n"
		" # ============================================================================= #\n"
		"###################################################################################\n";
	Log(LOG_FORCE, "\n%s" , screen);
}

static void* _pb_alloc(void* alloc_data , size_t size) {
	AVOID_NOUSED(alloc_data);
	if (size <= 0)
		return NULL;
	return ylmalloc(size);
}
static void _pb_free(void* alloc_data , void* ptr) {
	AVOID_NOUSED(alloc_data);
	if (!ptr)
		return;
	ylfree(ptr);
}

void InitPBAllocator() {
	g_pb_allocator.alloc = _pb_alloc;
	g_pb_allocator.free = _pb_free;
	g_pb_allocator.tmp_alloc = NULL;
	g_pb_allocator.max_alloca = MAX_PACKAGE_SIZE;
	g_pb_allocator.allocator_data = NULL;
}

void ExecutorInit() {
	g_executor.clients = Map_Init(
									Map_DefaultIntCmpFunc,
									Map_DefaultIntDupFunc,
									Map_DefaultFreeFunc,
									_client_dup_func,
									Map_DefaultFreeFunc
									);
	g_executor.cur_recv_buffer_ = 0;
	g_executor.conf_lock_ = WRLock_Init();
	for(int i = 0; i != CLIENT_RECV_QUEUES; ++i) {
		g_executor.recv_buffer_[i] = Queue_Init(sizeof(Message_t));
		sem_init(&g_executor.recv_buffer_num_[i] , 0 , 0);
	}
	g_executor.cur_send_buffer_ = 0;
	for(int i = 0; i != CLIENT_SEND_QUEUES; ++i) {
		g_executor.send_buffer_[i] = Queue_Init(sizeof(Message_t));
		sem_init(&g_executor.send_buffer_num_[i] , 0, 0);
	}
	g_executor.evloop = Event_CreateLoop(MAX_CLIENTS);
	if (g_executor.evloop == NULL) {
		fprintf(stderr, "[ERROR]Create server loop error.\n");
		exit(2);
	}
	g_executor.core_fd = netListenAndBind(NULL, g_executor.port_);
	if (g_executor.core_fd == ERR_NETWORK_EXCEPTION) {
		fprintf(stderr , "[ERROR]Create server error. Server quit. Specific binding port is [%d]\n" , g_executor.port_);
		exit(2);
	}
	if (netSetNonBlock(NULL, g_executor.core_fd) != NET_OK) {
		fprintf(stderr , "[ERROR]Set server main fd to non block failed\n");
		exit(2);
	}
	if ( EV_ERR == Event_CreateFileEvent(g_executor.evloop , g_executor.core_fd , EV_READABLE , _request_accept_func , NULL ) ) {
		fprintf(stderr, "[ERROR]Create server main event failed\n");
		exit(2);
	}
	InitPBAllocator();
	_executor_start_printscreen();
	Log(LOG_FORCE,"Executor Running at 0.0.0.0:%d",g_executor.port_);
	Log(LOG_FORCE,"Logs will save to \"%s\"" , g_executor.log_path_);
	Log(LOG_FORCE,"Task result will save to \"%s:%d\"" , g_executor.store_server_addr_ , g_executor.store_server_port_);
	Log(LOG_FORCE,"The CallBack URL for a finish task is [%s]" , g_executor.save_result_to);
	if (InitAndStartTaskMonitor(DefaultFuncOnTaskFinish)) {
		exit(2);
	}
	Log(LOG_FORCE,"Task Executor Start...");
}


void* _recv_buffer_monitor(void* arg) {
	int buf_idx = *(int*)arg;
	Message_t msg;
	for(;;) {
		sem_wait(&g_executor.recv_buffer_num_[buf_idx]);
		if (Queue_Pop(g_executor.recv_buffer_[buf_idx] , &msg) == QUEUE_OK) {
			DistributeMsg(&msg);
			ylfree(msg.package);
		}
	}
	return NULL;
}

#define MAX_SEND_TRY 500
void* _send_buffer_monitor(void* arg) {
	int buf_idx = *(int*)arg;
	Message_t msg;
	Client_t* c;
	for (;;) {
		sem_wait(&g_executor.send_buffer_num_[buf_idx]);
		if (Queue_Pop(g_executor.send_buffer_[buf_idx], &msg) == QUEUE_OK) {
			if ((c = Map_GetPtr(g_executor.clients, &msg.sockfd)) == NULL) {
				//Clients already gone
				Log(LOG_NOTICE,"[SEND FAIL]Client Already Gone");
				ylfree(msg.package);
				continue;
			}
			if (msg.trysend_cnt) {
				usleep(100000);
			}
			int nsend = netWrite(NULL, msg.sockfd, (char*) msg.package, msg.pkg_len);
			if (nsend == ERR_NETWORK_TRY_AGAIN&& ++msg.trysend_cnt < MAX_SEND_TRY) {
				Queue_Push(g_executor.send_buffer_[buf_idx] , &msg);
				sem_post(&g_executor.send_buffer_num_[buf_idx]);
			} else {
				ylfree(msg.package);
			}
		}
	}
	return NULL;
}

typedef struct {
	Queue_t clients_tobe_clear;
	time_t check_time;
	int		pluse_interval;
}client_iterator_t;

static void _foreach_client_(const void* key, const void* data , void* args) {
	client_iterator_t* iterator = (client_iterator_t*)args;
	Queue_t clients_tobe_clear = iterator->clients_tobe_clear;
	time_t now_time = iterator->check_time;
	int pluse_interval = iterator->pluse_interval;
	if (now_time - ((Client_t*)data)->last_seen > pluse_interval) {
		int client_fd = *((int*)key);
		Queue_Push(clients_tobe_clear , &client_fd);
	}
}
void* _client_monitor(void* arg) {
	AVOID_NOUSED(arg);
	client_iterator_t iterator;
	iterator.clients_tobe_clear = Queue_Init(sizeof(int));
	for(;;) {
		int interval = 0;
		WRLock_LockR(g_executor.conf_lock_);
		interval = g_executor.pluse_interval_;
		WRLock_UnlockR(g_executor.conf_lock_);
		iterator.pluse_interval = interval;
		iterator.check_time = time(NULL);
		Map_Foreach(g_executor.clients , _foreach_client_ , &iterator);
		int cfd;
		while(Queue_Pop(iterator.clients_tobe_clear , &cfd) == QUEUE_OK) {
			_release_client(cfd , RELEASE_BY_PLUSE);
		}
		sleep(interval);
	}
	return NULL;
}
void ExecutorStart() {
	pthread_t* send_threads = ylmalloc(sizeof(pthread_t) * CLIENT_SEND_QUEUES);
	int*	  n_send_thread = ylmalloc(sizeof(int) * CLIENT_SEND_QUEUES);
	pthread_t* recv_threads = ylmalloc(sizeof(pthread_t) * CLIENT_RECV_QUEUES);
	int*	  n_recv_thread = ylmalloc(sizeof(int) * CLIENT_RECV_QUEUES);
	pthread_t client_thread;
	for(int i = 0; i != CLIENT_SEND_QUEUES; ++i) {
		n_send_thread[i] = i;
		pthread_create(&send_threads[i] , NULL , _send_buffer_monitor , &n_send_thread[i]);
	}
	for(int i = 0; i != CLIENT_RECV_QUEUES; ++i) {
		n_recv_thread[i] = i;
		pthread_create(&recv_threads[i] , NULL , _recv_buffer_monitor , &n_recv_thread[i]);
	}
	pthread_create(&client_thread , NULL , _client_monitor , NULL);
	Event_Main(g_executor.evloop);
	Event_DestroyLoop(g_executor.evloop);
}
static void _make_header(uint8_t* head , int32_t msgid, int32_t msglen) {
	memset(head , 0 , MSGHEAD_SIZE);
	*head = 'v';
	*(head + 1) = '0';
	*(head + 2) = '0';
	*(head + 3) = '1';
	*((int32_t*)head + 1) = msgid;
	*((int32_t*)head + 2) = msglen;
}
void ReplyMessage(int fd , int msgid , int pkg_len, uint8_t* package) {
	Message_t new_msg;
	new_msg.sockfd = fd;
	new_msg.trysend_cnt = 0;
	new_msg.msgid = msgid;
	new_msg.package = package;
	new_msg.pkg_len = pkg_len;
	_make_header(new_msg.package , msgid , pkg_len - MSGHEAD_SIZE);
	uint8_t send_buf = atomic_add_then_fetch((int*) &g_executor.cur_send_buffer_, 1) % CLIENT_SEND_QUEUES;
	Queue_Push(g_executor.send_buffer_[send_buf], &new_msg);
	sem_post(&g_executor.send_buffer_num_[send_buf]);
}















