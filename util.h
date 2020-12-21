#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>

/* util
 * Random utilities. */

// Macros
#define eprintf(...) fprintf (stderr, __VA_ARGS__)
#define count(array, type) ((sizeof(array)) / (sizeof(type)))
#define strcount(array) count(array, char*)

typedef struct stringlist {
	char* content;
	struct stringlist* next;
} STRINGLIST;

char* heapstr(char* str, int len);
char* ezheapstr(char* str);
int countplaces(int n);
char* itoa(int i);
void* copy(void* v, int sz);

void printstrlist(STRINGLIST* strlist, FILE* stream);
void freestrlist(STRINGLIST* strlist);
#endif
