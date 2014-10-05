/*
 * test_store.c
 *
 *  Created on: Jul 19, 2013
 *      Author: House_Lee
 */

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "store.h"


#define MAX_KEY_LEN 20
char char_element[52] = {
		'a','b','c','d','e','f','g',
		'h','i','j','k','l','m','n',
		'o','p','q','r','s','t',
		'u','v','w','x','y','z',
		'A','B','C','D','E','F','G',
		'H','I','J','K','L','M','N',
		'O','P','Q','R','S','T',
		'U','V','W','X','Y','Z',
//		'0','1','2','3','4','5','6','7','8','9',
//		'!','@','#','$','%','*','&','-','+','?',
};
void GenerateRandomString(char src[] , int seed) {
	int len = rand()%MAX_KEY_LEN + 1;
	int i;
	for(i = 0; i != len ; ++i) {
		srand((unsigned)time(NULL) + i + seed);
		src[i] = char_element[rand()%52];
	}
	src[len] = '\0';
}

int main() {
	StoreInit("10.15.154.15" , 17016);

	printf("set foo bar\n");
	StoreSet("foo" , "bar");
	char* res = StoreGet("foo");
	if (res) {
		printf("get foo:%s\n",res);
	} else {
		printf("get foo failed\n");
	}
	StoreFree(res);
	char str[MAX_KEY_LEN + 1];
	for (int i = 0; i != 20; ++i) {
		GenerateRandomString(str, i);
		printf("set %s x\n", str);
		StoreSet(str , "x");
		printf("get %s:", str);
		res =  StoreGet(str);
		if (res) {
			printf("%s\n" , res);
		} else {
			printf("fail\n");
		}
		StoreFree(res);
		usleep(1000);
	}
	for (int i = 0; i != 32; ++i) {
		StoreAppend("test" , "x");
	}
	res = StoreGet("test");
	if (res) {
		printf("[%d]%s\n" , strlen(res) , res);
	} else {
		printf("get test failed\n");
	}
	StoreFree(res);

	StoreSetBinary("abc" , "qwertyuiop" , 10);
	char* out;
	int len;
	StoreGetBinary("abc" , (void**)&out , &len);
	printf("abc: [%d]%s\n" , len, out);
	StoreFree(out);
}
