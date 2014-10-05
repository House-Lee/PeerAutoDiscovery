/*
 * main.c
 *
 *  Created on: Jul 12, 2013
 *      Author: House_Lee
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "log.h"
#include "executor.h"


void daemonize(void) {
	if (fork() != 0)
		exit(0); /* parent exits */
	setsid(); /* create a new session */

	int fd;
	if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if (fd > STDERR_FILENO)
			close(fd);
	}
}

int main(int argc , char** argv) {
	int res;
	if (argc == 2) {
		res = ExecutorLoadConfig(argv[1]);
	} else {
		res = ExecutorLoadConfig(NULL);
	}
	if (res) {
		printf("[%d]Load configure failed\n" , res);
		exit(res);
	}
	if (g_executor.daemonize) {
		daemonize();
	}
	if ((res = ExcutorInitSingalHandlers())) {
		printf("Init Task Signal Handler failed\n");
		exit(res);
	}
	Log_Start(g_executor.log_path_ , g_executor.log_level , g_executor.daemonize);
	ExecutorInit();
	ExecutorStart();
}
