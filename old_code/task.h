/*
 * task.h
 *
 *  Created on: Jul 17, 2013
 *      Author: House_Lee
 */

#ifndef EXECUTOR_TASK_H_
#define EXECUTOR_TASK_H_

#include "../includes/map.h"
#include "../includes/queue.h"
#include "../includes/event.h"

//the maximum executing time for every task is 3min
#define MAX_TASK_EXE_TIME 1800

#ifdef	__cplusplus
extern "C" {
#endif

enum TASK_RESULT {
	TASK_SUCCESS = 0,
	TASK_FAIL,
};
enum TASK_STATUS {
	TASK_CREATE = 1,
	TASK_RUNNING,
	TASK_FINISH,
	TASK_TERM,
};


void _sig_chld_handler_(int sig);

typedef void (*onTaskFinFunc)(char* task_id , int retcode , int result, char* str_result , char* output);

typedef struct TASK_MANAGER{
	//configure
	int 			max_concurrent_task_;
	onTaskFinFunc	_on_task_finish_;
	char*			save_result_call_back_;

	//working scheduling
	Map_t			tasks_;
	Queue_t			pending_tasks_;
//	Map_t			running_tasks_fd_;//idx by recvfd
	Map_t			running_tasks_;//idx by pid
	volatile int 	n_running_;
	Queue_t			finished_tasks_;
	EventLoop_t 	evloop_;
}Task_Manager_t;
extern Task_Manager_t g_taskmanager;

void DefaultFuncOnTaskFinish(char* task_id , int retcode , int result, char* str_result , char* output);

int InitAndStartTaskMonitor(onTaskFinFunc task_callback);
int	Task_Create(const char* task_id , const char* command);
char* Task_GetOutput(const char* task_id);
int Task_GetResult(const char* task_id , int* result_out, char** str_out);
void Task_FreeResult(char* out);


#ifdef	__cplusplus
}
#endif

#endif /* EXECUTOR_TASK_H_ */
