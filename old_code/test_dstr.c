/*
 * test_dstr.c
 *
 *  Created on: Jul 18, 2013
 *      Author: House_Lee
 */

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "../includes/dstr.h"

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
/*
	char str[MAX_KEY_LEN + 1];
	DStr_t s = DStrNew("foo");
	printf("[%zu]%s\n" , DStrlen(s) , s);
	s = DStrAppend(s , "bar");
	printf("[%zu]%s\n" , DStrlen(s) , s);
	for(int i = 0; i != 20; ++i) {
		GenerateRandomString(str, i);
		printf("RandomString is:[%zu]%s\n" , strlen(str) , str);
		s = DStrAppend(s , str);
		printf("New s:[%zu]%s\n" , DStrlen(s) , s);
		usleep(1000);
	}
	*/
	/*
	DStr_t s = DStrNewEmpty(100);
	for(int i = 0; i != 52; ++i) {
		s = DStrAppend(s , "xx");
	}
	printf("[%zu]%s\n" , DStrlen(s) , s);
	*/
	char str[MAX_KEY_LEN + 1];
	DStr_t s = DStrNew("foo");
	printf("[%zu][%zu]%s\n", DStrlen(s),DStrAvailLen(s), s);
	s = DStrAssign(s, "bar");
	printf("[%zu][%zu]%s\n", DStrlen(s),DStrAvailLen(s), s);
	for (int i = 0; i != 20; ++i) {
		GenerateRandomString(str, i);
		printf("RandomString is:[%zu]%s\n", strlen(str), str);
		s = DStrAssign(s, str);
		printf("New s:[%zu][%zu]%s\n", DStrlen(s), DStrAvailLen(s),s);
		usleep(1000);
	}

}
