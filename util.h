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
	const char* content;
	struct stringlist* next;
} STRINGLIST;

typedef struct {
	const char** items;
	const int size;
} STRINGARRAY;

char* heapstr(const char* str, int len);
char* ezheapstr(const char* str);
int countplaces(int n);
char* itoa(int i);
void* copy(void* v, int size);

STRINGLIST* initstrlist(const char** strs, int count);
void printstrlist(STRINGLIST* strlist, FILE* stream);
void freestrlist(STRINGLIST* strlist);
#endif
