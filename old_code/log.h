/*
 * log.h
 *
 *  Created on: Jul 12, 2013
 *      Author: House_Lee
 */

#ifndef EXECUTOR_LOG_H_
#define EXECUTOR_LOG_H_

#define MAX_LOG_ITEM_LEN 1024

#ifdef	__cplusplus
extern "C" {
#endif

extern char* log_level_map[];

void Log_Start(const char* logfile , int log_level , int daemonize);
int Log_GetLevel();
void Log_SetLevel(int loglevel);
void Log(int log_level, const char* fmt, ...)__attribute__((format(printf , 2 , 3)));

#ifdef	__cplusplus
}
#endif

#endif /* LOG_H_ */
