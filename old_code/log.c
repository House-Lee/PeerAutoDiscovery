/*
 * log.c
 *
 *  Created on: Jul 12, 2013
 *      Author: House_Lee
 */
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include "commual.h"
#include "../includes/queue.h"
#include "../includes/util.h"
#include "../includes/lock.h"
#include "log.h"

#define LOG_STARTED MAGIC_CODE

typedef struct LOG_ITEM {
	int level;
	char* content;
}Log_Item;

typedef struct LOG {
	int			log_level;
	int			start_;
	bool		daemonize;
	char*		path_;
	Queue_t 	queue_;
	uint32_t	cnt_;
	WRLock_t	conf_lock_;
}Log_t;

static Log_t g_log;

char* log_level_map[] = {
		"",
		"[-]",
		"[ERROR]",
		"[WARNING]",
		"[NOTICE]",
		"[DEBUG]",
		""
};

static void* monitor_log(void* arg) {
	AVOID_NOUSED(arg);
	FILE* fp;
	int cur_cnt;
	for(;;) {
		Log_Item incomming_log;
		int conf_log_level;
		WRLock_LockR(g_log.conf_lock_);
		conf_log_level = g_log.log_level;
		WRLock_UnlockR(g_log.conf_lock_);

		while(Queue_Pop(g_log.queue_ , &incomming_log) == QUEUE_OK) {
			if (incomming_log.level >= LOG_LEV_END || incomming_log.level <= LOG_LEV_START) {
				incomming_log.level = LOG_DEBUG;
			}
			if (incomming_log.level <= conf_log_level) {
				cur_cnt = atomic_add_then_fetch((int*) &g_log.cnt_, 1);
				fp = g_log.path_ ? fopen(g_log.path_, "a") : NULL;
				if (!g_log.daemonize) {
					printf("%09d: %s%s", cur_cnt,log_level_map[incomming_log.level],incomming_log.content);
					fflush(stdout);
				}
				if (fp) {
					fprintf(fp, "%09d: %s%s", cur_cnt,log_level_map[incomming_log.level], incomming_log.content);
					fflush(fp);
					fclose(fp);
				}
			} else {
				if (!g_log.daemonize) {
					printf("---------: %s%s",log_level_map[incomming_log.level], incomming_log.content);
				}
			}
			ylfree(incomming_log.content);
		}
		usleep(10000);
	}
	return NULL;
}
void Log_Start(const char* logfile , int log_level , int daemonize) {
	g_log.start_ = LOG_STARTED;
	g_log.queue_ = Queue_Init(sizeof(Log_Item));
	g_log.cnt_ = 0;
	g_log.daemonize = daemonize;
	g_log.path_ = (char*)logfile;
	g_log.conf_lock_ = WRLock_Init();
	Log_SetLevel(log_level);
	pthread_t monitor_thread;
	pthread_create(&monitor_thread , NULL , monitor_log , NULL);
}
int Log_GetLevel() {
	int rtn;
	WRLock_LockR(g_log.conf_lock_);
	rtn = g_log.log_level;
	WRLock_UnlockR(g_log.conf_lock_);
	return rtn;
}
void Log_SetLevel(int loglevel) {
	if (loglevel >= LOG_LEV_END || loglevel <= LOG_LEV_START) {
		return;
	}
	WRLock_LockW(g_log.conf_lock_);
	g_log.log_level = loglevel;
	WRLock_UnlockW(g_log.conf_lock_);
}

void Log(int log_level , const char* fmt, ...) {
	if (g_log.start_ != LOG_STARTED) {
		fprintf(stderr, "LOG NOT STARTED\n");
		return;
	}
	char strtime[64];
	int offset;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	offset = strftime(strtime, sizeof(strtime), "%d %b %H:%M:%S.",
			localtime(&tv.tv_sec));
	snprintf(strtime + offset, sizeof(strtime) - offset,
			"%03d", (int)tv.tv_usec/1000);

	va_list ap;
	char msg[MAX_LOG_ITEM_LEN];
	va_start(ap , fmt);
	vsnprintf(msg , MAX_LOG_ITEM_LEN , fmt , ap);
	va_end(ap);
	int len = strlen(strtime) + strlen(msg) + 10;
	char* real_log = ylmalloc(len);
	snprintf(real_log , len , "[%s] %s\n" , strtime , msg);
	Log_Item new_log;
	new_log.level = log_level;
	new_log.content = real_log;
	Queue_Push(g_log.queue_ , &new_log);
}

