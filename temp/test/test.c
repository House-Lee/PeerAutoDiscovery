#include <stdio.h>

#ifndef true
#define true (1)
#endif /* ! true */

#ifndef false
#define false (0)
#endif /* ! false */

typedef int bool;

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


int main() {
	printf("%d\n" , is_numeric("+"));
	printf("%d\n" , is_numeric("+13.5"));
	printf("%d\n" , is_numeric("-12"));
	printf("%d\n" , is_numeric("12.5.3"));
	printf("%d\n" , is_numeric("218"));
	printf("%d\n" , is_numeric("32.5"));
	printf("%d\n" , is_numeric("-13.5"));
}

