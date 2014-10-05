/*
 * task.c
 *
 *  Created on: Jul 17, 2013
 *      Author: House_Lee
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <stdint.h>
#include <curl/curl.h>

#include "task.h"
#include "commual.h"
#include "executor.h"
#include "store.h"
#include "log.h"
#include "../includes/lock.h"
#include "../includes/memory.h"
#include "../includes/dstr.h"
#include "../includes/network.h"

#include "../protos/compiled/common_msg.pb-c.h"

Task_Manager_t g_taskmanager;

#define DEFAULT_TASK_OUTPUT_LEN 2048
#define TASK_BUFFER_LEN 1024
#define READ_PORT 0
#define WRITE_PORT 1

typedef struct TASK_DESC {
	//basic attr
	char*	task_id_;
	char*	cmd_;

	//system
	pid_t				process_id_;
	int					recv_fd_;
	int					recv_buf_len_;
	char* 				recv_buf_;
	time_t				start_time_;
	time_t				end_time_;
	int					param_cnt_;
	char**				params_;

	//running record
	WRLock_t 			oplock;
	enum TASK_STATUS	status;
	int					retcode;
	DStr_t				output;
	int					result;
	char*				str_result;
} Task_t;


static void* _task_real_dup(const void* tsk) {
	Task_t* newtsk = ylmalloc(sizeof(Task_t));
	memcpy(newtsk , tsk , sizeof(Task_t));
	return newtsk;
}
static void* _task_pid_dup(const void* pid) {
	pid_t* newpid = ylmalloc(sizeof(pid_t));
	memcpy(newpid , pid , sizeof(pid_t));
	return newpid;
}
static void* _task_running_dup(const void* tsk) {
	return (void*)tsk;
}
static void _task_running_free(void* tsk) {
	AVOID_NOUSED(tsk);
	return;
}

static void _close_chld_tsk_nolock(Task_t* tsk, int status , int retcode) {
	tsk->status = status;
	tsk->retcode = retcode;
	tsk->end_time_ = time(NULL);
	Event_DeleteFileEvent(g_taskmanager.evloop_ , tsk->recv_fd_ , EV_READABLE);
//	Map_Del(g_taskmanager.running_tasks_fd_ , &tsk->recv_fd_);
	close(tsk->recv_fd_);
}


typedef struct _run_tsk_args{
	time_t 	check_time;
	Queue_t	term_queue;
}_RunningCheck_t;

static void _foreach_running_task(const void* key, const void* data , void* args) {
	Task_t* tsk = (Task_t*)data;
	_RunningCheck_t* param = (_RunningCheck_t*)args;
	if (param->check_time - tsk->start_time_ > MAX_TASK_EXE_TIME) {
		WRLock_LockR(tsk->oplock);
		Queue_Push(param->term_queue , (pid_t*)key);
		WRLock_UnlockR(tsk->oplock);
	}
}
#define CONTAIN_RESULT true
static bool _digest_task_buffer_nolock(Task_t* tsk) {
	int contain_res = false;
	char* res_str = strstr(tsk->recv_buf_ , "[YUNLU-RESULT]");
	int cpyend = 0;
	if (!res_str) {
		cpyend = tsk->recv_buf_len_;
	} else {
		cpyend = res_str - tsk->recv_buf_;
		char* str_result = res_str + 14;//14 is strlen("[YUNLU-RESULT]")
		char* end_flag = strstr(str_result , "\r\n");
		if (end_flag) {
			cpyend = tsk->recv_buf_len_;
			contain_res = true;
			if (end_flag - str_result < 4) {
				tsk->result = TASK_FAIL;
				tsk->str_result = ylstrdup("Result not correct");
			} else {
				if (!strncmp(str_result , "SUCC" , 4)) {
					tsk->result = TASK_SUCCESS;
				} else {
					tsk->result = TASK_FAIL;
				}
				str_result += 4;
				if (*str_result == ':')
					++str_result;
				int len = end_flag - str_result;
				tsk->str_result = ylmalloc(len + 1);
				memcpy(tsk->str_result , str_result , len);
				tsk->str_result[len] = '\0';
			}
		}
	}
	tsk->output = DStrNAppend(tsk->output , tsk->recv_buf_ , cpyend);
	tsk->recv_buf_len_ -= cpyend;
	for (int i = 0; i != tsk->recv_buf_len_; ++i) {
		tsk->recv_buf_[i] = tsk->recv_buf_[cpyend + i];
	}
	return contain_res;
}
static void _release_task(Task_t* tsk) {
	ylfree(tsk->task_id_);
	ylfree(tsk->cmd_);
	for (int i = 0 ;i != tsk->param_cnt_; ++i) {
		ylfree(tsk->params_[i]);
	}
	ylfree(tsk->params_);
	ylfree(tsk->recv_buf_);
	WRLock_Destroy(tsk->oplock);
	DStrFree(tsk->output);
	ylfree(tsk->str_result);
	ylfree(tsk);
}

static void _recv_msg_from_child(EventLoop_t evloop , int fd , void* clientData , int mask) {
	AVOID_NOUSED(evloop);
	AVOID_NOUSED(mask);
	int nread;
	Task_t* tsk = (Task_t*)clientData;
	WRLock_LockW(tsk->oplock);
	nread = read(fd , tsk->recv_buf_ + tsk->recv_buf_len_, TASK_BUFFER_LEN - tsk->recv_buf_len_);
	if (nread < 0) {
		Log(LOG_ERROR,"RECV FROM CHILD. {\"task id\":\"%s\", \"cmd\":\"%s\",\"errmsg\":\"%s\"}", tsk->task_id_ , tsk->cmd_ , strerror(errno));
		kill(tsk->process_id_ , SIGTERM);
		WRLock_UnlockW(tsk->oplock);
		return;
	}
	tsk->recv_buf_len_ += nread;
	tsk->recv_buf_[tsk->recv_buf_len_] = '\0';
	if (_digest_task_buffer_nolock(tsk) == CONTAIN_RESULT) {
		kill(tsk->process_id_ , SIGTERM);
	}
	WRLock_UnlockW(tsk->oplock);
}
static int _execute_task(Task_t* task) {
//1.change tsk->status to TASK_RUNNING
//2.fork new process to run and mark tsk->start_time
//3.call _recv_msg_from_child
	if (task->status != TASK_CREATE) {
		return ERR_TASK_NOT_CREATED;
	}
	int fd[2];
	pid_t process_id;
	task->start_time_ = time(NULL);
	if (pipe(fd) < 0) {
		return ERR_TASK_CREATE_PIPE_FAIL;
	}
	if ( (process_id = fork()) < 0) {
		close(fd[READ_PORT]);
		close(fd[WRITE_PORT]);
		return ERR_TASK_FORK_PROCESS_FAIL;
	}

	if (process_id > 0) {
		close(fd[WRITE_PORT]);
		task->process_id_ = process_id;
		task->status = TASK_RUNNING;
		task->recv_fd_ = fd[READ_PORT];
		Map_Set(g_taskmanager.running_tasks_, &process_id, task);
		netSetNonBlock(NULL , fd[READ_PORT]);
		Event_CreateFileEvent(g_taskmanager.evloop_, fd[READ_PORT], EV_READABLE,_recv_msg_from_child, task);
	} else {
		close(fd[READ_PORT]);
		dup2(fd[WRITE_PORT] , STDOUT_FILENO);
		dup2(fd[WRITE_PORT] , STDERR_FILENO);
		chdir("/root/");
		if (execvp(task->params_[0], (char* const*)task->params_) < 0) {
			printf("[YUNLU-RESULT]FAIL:Execute [%s] error\r\n" , task->cmd_);
			exit(1);
		}
	}
	return 0;
}

static void* _event_monitor_thread(void* arg) {
	AVOID_NOUSED(arg);
	Event_Main(g_taskmanager.evloop_);
	Event_DestroyLoop(g_taskmanager.evloop_);
	return NULL;
}

static void* _task_monitor_thread(void* arg) {
	AVOID_NOUSED(arg);
	_RunningCheck_t running_check;
	running_check.term_queue = Queue_Init(sizeof(pid_t));
	for(;;) {
		usleep(10000);
		//check all running task to see whether any of them had been running out-of-time
		running_check.check_time = time(NULL);
		Map_Foreach(g_taskmanager.running_tasks_ , _foreach_running_task , &running_check);
		pid_t tsk_to_be_terminated;
		while(Queue_Pop(running_check.term_queue , &tsk_to_be_terminated) == QUEUE_OK) {
			kill(tsk_to_be_terminated , SIGTERM);
		}
		//process all finished task
		Task_t* tsk;
		while(Queue_Pop(g_taskmanager.finished_tasks_ , &tsk) == QUEUE_OK ) {
			--g_taskmanager.n_running_;
			tsk = Map_GetAndDel(g_taskmanager.tasks_ , tsk->task_id_);
			WRLock_LockW(tsk->oplock);
			_digest_task_buffer_nolock(tsk);
			if (tsk->recv_buf_len_ > 0) {
				tsk->output = DStrNAppend(tsk->output , tsk->recv_buf_ , tsk->recv_buf_len_);
			}
			if (!tsk->str_result) {
				tsk->result = TASK_FAIL;
				tsk->str_result = ylstrdup("[NOT SPECIFIED DURING EXECUTION]");
			}
			g_taskmanager._on_task_finish_(tsk->task_id_ , tsk->retcode , tsk->result , tsk->str_result , (char*)tsk->output);
			WRLock_UnlockW(tsk->oplock);
			_release_task(tsk);
		}
		//start as many new task as possible
		while (g_taskmanager.n_running_ < g_taskmanager.max_concurrent_task_) {
			if (Queue_Pop(g_taskmanager.pending_tasks_ , &tsk) != QUEUE_OK) {
				break;
			} else {
				++g_taskmanager.n_running_;
				_execute_task(tsk);
			}
		}
	}
	return NULL;
}

void _sig_chld_handler_(int sig) {
	AVOID_NOUSED(sig);
	int status;
	pid_t pid;
	while( (pid = waitpid(-1 ,&status , WNOHANG)) > 0) {
		Task_t* tsk = Map_GetAndDel(g_taskmanager.running_tasks_ , &pid);
		if (tsk == NULL)
			continue;
		if (WIFEXITED(status)) {
			WRLock_LockW(tsk->oplock);
			_close_chld_tsk_nolock(tsk , TASK_FINISH , 0);
			WRLock_UnlockW(tsk->oplock);
		} else {
			WRLock_LockW(tsk->oplock);
			_close_chld_tsk_nolock(tsk , TASK_TERM , WEXITSTATUS(status));
			WRLock_UnlockW(tsk->oplock);
		}
		Queue_Push(g_taskmanager.finished_tasks_ , &tsk);
	}
}

int InitAndStartTaskMonitor(onTaskFinFunc task_callback) {
	Log(LOG_FORCE,"InitAndStartTaskMonitor():Initializing...");
	g_taskmanager.max_concurrent_task_ = g_executor.max_concurrent_task_;
	g_taskmanager._on_task_finish_ = task_callback;
	g_taskmanager.save_result_call_back_ = g_executor.save_result_to;

	g_taskmanager.tasks_ = Map_Init(
									Map_DefaultStringCmpFunc,
									Map_DefaultStringDupFunc,
									Map_DefaultFreeFunc,
									_task_real_dup,
									Map_DefaultFreeFunc
									);
	g_taskmanager.pending_tasks_ = Queue_Init(sizeof(Task_t*));
//	g_taskmanager.running_tasks_fd_ = Map_Init(
//									Map_DefaultIntCmpFunc,
//									Map_DefaultIntDupFunc,
//									Map_DefaultFreeFunc,
//									_task_running_dup,
//									_task_running_free
//									);
	g_taskmanager.running_tasks_ = Map_Init(
									Map_DefaultIntCmpFunc,
									_task_pid_dup,
									Map_DefaultFreeFunc,
									_task_running_dup,
									_task_running_free
									);
	g_taskmanager.n_running_ = 0;
	g_taskmanager.finished_tasks_ = Queue_Init(sizeof(Task_t*));
	if ( (g_taskmanager.evloop_ = Event_CreateLoop(g_taskmanager.max_concurrent_task_)) == NULL) {
		Log(LOG_FORCE,"InitAndStartTaskMonitor():Create Event Listener Failed.");
		return ERR_TASK_MONITOR_INIT_FAIL;
	}
	Log(LOG_FORCE,"InitAndStartTaskMonitor():Starting Task Monitor...");
	pthread_t task_monitor;
	pthread_t event_monitor;
	pthread_create(&task_monitor , NULL , _task_monitor_thread , NULL);
	pthread_create(&event_monitor , NULL , _event_monitor_thread , NULL);
	if (StoreInit(g_executor.store_server_addr_ ,g_executor.store_server_port_)) {
		Log(LOG_FORCE,"[CAUTION]InitAndStartTaskMonitor():connect to storage server failed, result might not be saved!");
	}
	Log(LOG_FORCE,"InitAndStartTaskMonitor():Finish");
	return 0;
}

int	Task_Create(const char* task_id , const char* command) {
	if (Map_GetPtr(g_taskmanager.tasks_ , task_id) != NULL) {
		return ERR_TASK_ALREADY_RUNNING;
	}
	Task_t task;
	memset(&task , 0 , sizeof(Task_t));
	task.task_id_ = ylstrdup(task_id);
	task.cmd_ = ylstrdup(command);

	char* start = task.cmd_;
	char* end = task.cmd_ + strlen(task.cmd_);
	while (*start == ' ') ++start;
	while (*(--end) == ' ') ;
	*(++end) = '\0';
	if (start >= end) {
		ylfree(task.task_id_);
		ylfree(task.cmd_);
		return ERR_TASK_PARAM;
	}

	task.recv_buf_ = ylmalloc((TASK_BUFFER_LEN + 1) * sizeof(char));
	task.oplock = WRLock_Init();
	task.output = DStrNewEmpty(DEFAULT_TASK_OUTPUT_LEN);
	task.result = task.retcode = MAGIC_CODE;
	task.str_result = NULL;


	task.param_cnt_ = 1;
	char* iterator;
	for(iterator = start; iterator < end; ++iterator) {
		if (*iterator == ' ') {
			*iterator = '\0';
			++task.param_cnt_;
			while ((iterator <= end) && (*iterator == ' ')) ++iterator;
		}
	}
	task.params_ = ylmalloc((task.param_cnt_ + 1)*sizeof(char*) );
	iterator = start;
	for(int i = 0; i != task.param_cnt_; ++i) {
		task.params_[i] = ylstrdup(iterator);
		iterator += strlen(task.params_[i]);
		while(*(++iterator) == ' ');
	}
	task.params_[task.param_cnt_] = NULL;
	task.status = TASK_CREATE;
	Task_t* stored_tsk = Map_SetAndGetPtr(g_taskmanager.tasks_ , task_id ,  &task);
	Queue_Push(g_taskmanager.pending_tasks_ , &stored_tsk);
	return 0;
}
char* Task_GetOutput(const char* task_id) {
	Task_t* task = Map_GetPtr(g_taskmanager.tasks_ , task_id);
	char* rtn = NULL;
	if (task == NULL) {
		char* taskpkg;
		int pkglen;
		if (StoreGetBinary(task_id , (void**)&taskpkg , &pkglen)) {
			return NULL;
		}
		TaskDesc* pkg = task__desc__unpack(&g_pb_allocator , pkglen , (uint8_t*)taskpkg);
		if (!pkg) {
			return NULL;
		}
		rtn = ylstrdup(pkg->output);
		task__desc__free_unpacked(pkg , &g_pb_allocator);
	} else {
		WRLock_LockR(task->oplock);
		rtn = ylstrdup(task->output);
		WRLock_UnlockR(task->oplock);
	}
	return rtn;
}
int Task_GetResult(const char* task_id , int* result_out, char** str_out) {
	if (!str_out || !result_out) {
		return ERR_TASK_RESULT_PARAM;
	}
	*str_out = NULL;
	*result_out = MAGIC_CODE;
	Task_t* task = Map_GetPtr(g_taskmanager.tasks_ , task_id);
	if (task == NULL) {
		char* taskpkg;
		int pkglen;
		if (StoreGetBinary(task_id, (void**)&taskpkg, &pkglen)) {
			return ERR_TASK_RESULT_NOT_FOUND;
		}
		TaskDesc* pkg = task__desc__unpack(&g_pb_allocator, pkglen,(uint8_t*) taskpkg);
		if (!pkg) {
			return ERR_TASK_RESULT_FORMAT_ERR;
		}
		*str_out = ylstrdup(pkg->str_result);
		*result_out = pkg->result;
	} else {
		WRLock_LockR(task->oplock);
		if (task->str_result) {
			*str_out = ylstrdup(task->str_result);
		} else {
			*str_out = ylstrdup("no result yet");
		}
		*result_out = task->result;
		WRLock_UnlockR(task->oplock);
	}
	return 0;
}
void Task_FreeResult(char* out) {
	ylfree(out);
}

static size_t default_blocked_writedata_func(void* buffer , size_t size , size_t nmemb , void* userp) {
	return nmemb;
}
static void _post_back_(char* pkg, int len) {
	CURL* curl_handler = curl_easy_init();
	struct curl_slist* header = NULL;
	header = curl_slist_append(header, "Content-Type: text/xml");
	curl_easy_setopt(curl_handler, CURLOPT_HTTPHEADER, header);
	curl_easy_setopt(curl_handler, CURLOPT_URL,g_taskmanager.save_result_call_back_);
	curl_easy_setopt(curl_handler, CURLOPT_POSTFIELDSIZE_LARGE, len);
	curl_easy_setopt(curl_handler, CURLOPT_POSTFIELDS, pkg);
	curl_easy_setopt(curl_handler, CURLOPT_WRITEFUNCTION,default_blocked_writedata_func);
	curl_easy_perform(curl_handler);
	curl_slist_free_all(header);
	curl_easy_cleanup(curl_handler);
}
void DefaultFuncOnTaskFinish(char* task_id , int retcode , int result, char* str_result , char* output) {
	TaskDesc task = TASK__DESC__INIT;
	task.task_id = task_id;
	task.retcode = retcode;
	task.result = result;
	task.str_result = str_result;
	task.output = output;
	int pkglen = task__desc__get_packed_size(&task);
	char* pkg = ylmalloc(pkglen);
	task__desc__pack(&task , (uint8_t*)pkg);
	StoreSetBinary(task_id , (void*)pkg , pkglen);
	_post_back_(pkg , pkglen);
	ylfree(pkg);
}

