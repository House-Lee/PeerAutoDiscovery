/*
 * executor.h
 *
 *  Created on: Jul 5, 2013
 *      Author: House_Lee
 */

#ifndef EXECUTOR_H_
#define EXECUTOR_H_

#include <stdint.h>
#include <semaphore.h>
#include "commual.h"
#include "../includes/queue.h"
#include "../includes/event.h"
#include "../includes/map.h"
#include "../includes/lock.h"

#define MAX_CONFIG_LINE 	256
#define MAX_CONFIG_LINE_LEN 255


#ifdef	__cplusplus
extern "C" {
#endif
typedef struct {
	//configure area
	bool			daemonize;
	int				pluse_interval_;//in 's'
	int				port_;
	char*			log_path_;//path+filename
	char			store_server_addr_[IP_ADDR_LEN];
	int				store_server_port_;
	int				max_concurrent_task_;
	char*			save_result_to;
	int				log_level;
	WRLock_t		conf_lock_;

	//runtime area
	int				core_fd;
	EventLoop_t		evloop;
	Map_t			clients;
	uint8_t 		cur_recv_buffer_;
	Queue_t			recv_buffer_[CLIENT_RECV_QUEUES];
	sem_t			recv_buffer_num_[CLIENT_RECV_QUEUES];
	uint8_t		 	cur_send_buffer_;
	Queue_t 		send_buffer_[CLIENT_SEND_QUEUES];
	sem_t			send_buffer_num_[CLIENT_SEND_QUEUES];
} Executor_t;
extern Executor_t g_executor;

int ExcutorInitSingalHandlers();
int ExecutorLoadConfig(const char* filename);
int ExecutorSetConfig(const char* key, const char* value);
char* ExecutorGetConfig(const char* key);
void ExecutorFreeResult(char* ptr);
void ExecutorInit();
void ExecutorStart();

#ifdef	__cplusplus
}
#endif

#endif /* EXECUTOR_H_ */
