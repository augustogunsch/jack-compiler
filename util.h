#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

char* heapstr(char* str, int len);
char* ezheapstr(char* str);
int countplaces(int n);

typedef struct line {
	char** tokens;
	int tokenscount;
	struct line* next;
} LINE;

typedef struct lnls {
	char* content;
	int truen;
	struct lnls* next;
} LINELIST;

void printlns(LINELIST* lns, FILE* stream);
void freelns(LINELIST* lns);
LINE* mkline(int count);
char* itoa(int i);
#endif
