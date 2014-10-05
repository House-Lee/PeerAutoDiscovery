/*
 * test_queue.c
 *
 *  Created on: Jul 9, 2013
 *      Author: House_Lee
 */

#include <stdio.h>
#include "../includes/queue.h"

int main() {
	Queue_t queue = Queue_Init(sizeof(int));
	for (int i = 0; i != 100; ++i) {
		Queue_Push(queue , &i);
	}
	int res;
	while(!Queue_Pop(queue , &res))
		printf("%5d" , res);
	printf("\n");
}
