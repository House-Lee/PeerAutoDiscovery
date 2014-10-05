/*
 * test_map.c
 *
 *  Created on: Jul 11, 2013
 *      Author: House_Lee
 */

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>


#include "../includes/map.h"
#define MAX_DATA 200
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
void foreach_callback(const void* key , const void* data , void* args) {
	printf("[%s][%d]\n",(char*)key,*((int*)data));
}
int main() {
	char str[MAX_DATA][MAX_KEY_LEN + 1];

	for (int i = 0; i != MAX_DATA; ++i) {
		GenerateRandomString(str[i], i);
		printf("the %5d test string generated: %s\n", i, str[i]);
		usleep(1000);
	}
	Map_t map = Map_Init(Map_DefaultStringCmpFunc,
						 Map_DefaultStringDupFunc,
						 Map_DefaultFreeFunc,
						 Map_DefaultIntDupFunc,
						 Map_DefaultFreeFunc
						);
	for(int i = 0; i != MAX_DATA; ++i) {
		printf("key[%s] val[%d]\n",str[i],i);
		Map_Set(map , str[i] , &i);
	}
	Map_Foreach(map , foreach_callback , NULL);
	for(int i = 0; i != 10; ++i) {
		int idx = rand()%MAX_DATA;
		printf("Copy Getting: %s --> " , str[idx]);
		int* res = Map_Get(map , str[idx]);
		printf("%d | " , *res);
		Map_FreeResult(map , res);
		int tmp = (int)time(NULL);
		Map_Set(map , str[idx] , &tmp);
		res = Map_GetPtr(map , str[idx]);
		printf("%d\n" , *res);
		sleep(1);
		srand((unsigned)time(NULL) + i + idx);
	}
	for (int i = 0; i != 10; ++i) {
		int idx = rand() % MAX_DATA;
		printf("Direct Getting: %s --> ", str[idx]);
		int* res = Map_GetPtr(map, str[idx]);
		printf("%d\n", *res);
		sleep(1);
		srand((unsigned)time(NULL) + i + idx);
	}
	Map_Destroy(map);
}
