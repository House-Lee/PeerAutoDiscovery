/*
 * util.c
 *
 *  Created on: Jul 9, 2013
 *      Author: House_Lee
 */


#include <sys/syscall.h>
#include <unistd.h>

#include "util.h"


bool is_numeric(const char* str) {
	char* iterator = (char*)str;
	if ((*iterator) == '+' || (*iterator) == '-') {
		++iterator;
	}
	if (!(*iterator)) {
		return false;
	}
	bool has_dot = false;
	while(*iterator) {
		if ((*iterator) > '9' || (*iterator) < '0') {
			if ((*iterator) == '.' && has_dot == false) {
				has_dot = true;
			} else {
				return false;
			}
		}
		++iterator;
	}
	return true;
}

int get_thread_id() {
#ifdef __linux__
	return (int)syscall(SYS_gettid);
#else
	return 0;
#endif
}
