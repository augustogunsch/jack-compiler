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

void* copy(void* v, int sz) {
	void* copy = malloc(sz);
	memcpy(copy, v, sz);
	return copy;
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

void printstrlist(STRINGLIST* strlist, FILE* stream) {
	while(strlist != NULL) {
		fprintf(stream, "%s\n", strlist->content);
		strlist = strlist->next;
	}
}

void freestrlist(STRINGLIST* strlist) {
	STRINGLIST* next = strlist->next;
	free(strlist);
	if(next != NULL)
		freestrlist(next);
}
