#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>

/* util
 * Random utilities. */

typedef struct stringlist {
	char* content;
	struct stringlist* next;
} STRINGLIST;

char* heapstr(char* str, int len);
char* ezheapstr(char* str);
int countplaces(int n);
char* itoa(int i);

void printstrlist(STRINGLIST* strlist, FILE* stream);
void freestrlist(STRINGLIST* strlist);
#endif
