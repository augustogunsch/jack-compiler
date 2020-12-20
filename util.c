#include <string.h>
#include <stdlib.h>
#include "util.h"

char* heapstr(char* str, int len) {
	int sz = sizeof(char) * (len + 1);
	char* outstr = (char*)malloc(sz);
	strcpy(outstr, str);
	return outstr;
}

char* ezheapstr(char* str) {
	return heapstr(str, strlen(str));
}

int countplaces(int n) {
	int places = 1;
	int divisor = 1;
	if(n < 0) {
		n = -n;
		places++;
	}
	while(n / divisor >= 10) {
		places++;
		divisor *= 10;
	}
	return places;
}

char* itoa(int i) {
	int sz = sizeof(char)*(countplaces(i)+1);
	char* a = (char*)malloc(sz);
	snprintf(a, sz, "%i", i);
	return a;
}

void printlns(LINELIST* lns, FILE* stream) {
	LINELIST* curln = lns;
	while(curln != NULL) {
		fprintf(stream, "%s\n", curln->content);
		curln = curln->next;
	}
}

void freelns(LINELIST* lns) {
	LINELIST* next = lns->next;
	free(lns);
	if(next != NULL)
		freelns(next);
}

LINE* mkline(int count) {
	LINE* l = (LINE*)malloc(sizeof(LINE));
	l->tokenscount = 0;
	l->tokens = (char**)malloc(sizeof(char*)*count);
	return l;
}
