/*
 * test_executor.c
 *
 *  Created on: Jul 9, 2013
 *      Author: House_Lee
 */


#include <stdio.h>
#include "executor.h"

int main() {
	int rtn;
	if ((rtn = ExecutorLoadConfig("tmp.conf")) ) {
		printf("ERROR:%d\n" , rtn);
		return rtn;
	}
	printf ("struct g_executor{\n");
	printf("\tdaemonize	:%d\n", g_executor.daemonize);
	printf("\tport		:%d\n", g_executor.port_);
	printf("\tlog path	:%s\n", g_executor.log_path_);
	printf("\tsto_serv	:%s\n", g_executor.store_server_addr_);
	printf("\tsto_port	:%d\n", g_executor.store_server_port_);
	printf("};\n");
}
