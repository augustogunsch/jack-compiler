#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "parser.h"
#include "compiler.h"
#include "io.h"

typedef struct unit {
	FILELIST* file;
	TOKEN* tokens;
	CLASS* parsed;
	COMPILER* compiler;
	LINEBLOCK* compiled;
	struct unit* next;
} COMPILEUNIT;

void* parseunit(void* input) {
	COMPILEUNIT* unit = (COMPILEUNIT*)input;

	unit->parsed = parse(unit->tokens, unit->file->name);

	pthread_exit(NULL);
}

void* compileunit(void* input) {
	COMPILEUNIT* unit = (COMPILEUNIT*)input;

	unit->compiled = compileclass(unit->compiler, unit->parsed);

	pthread_exit(NULL);
}

void waitthreads(pthread_t* threads, int amount) {
	void* status;
	int code;
	for(int i = 0; i < amount; i++) {
		code = pthread_join(threads[i], &status);
		if(code) {
			eprintf("Error while joining thread %i: %s\n", i, strerror(code));
			exit(code);
		}
	}
}

void actonunits(COMPILEUNIT* units, void*(*fun)(void*)) {
	pthread_t mythreads[_SC_THREAD_THREADS_MAX];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	COMPILEUNIT* curr = units;

	int i;
	int code;
	do {
		i = 0;
		while(curr != NULL && i < _SC_THREAD_THREADS_MAX) {
			code = pthread_create(&mythreads[i], NULL, fun, curr);

			if(code) {
				eprintf("Error while creating thread %i: %s\n", i, strerror(code));
				exit(code);
			}

			curr = curr->next;
			i++;
		}
		waitthreads(mythreads, i);
	} while(i == _SC_THREAD_THREADS_MAX);

	pthread_attr_destroy(&attr);
}

int main(int argc, char* argv[]) {

	if(argc < 2) {
		eprintf("Usage: %s {input file(s)}\n", argv[0]);
		return 1;
	}

	FILELIST* files = getfiles(argv[1]);
	FILELIST* curr = files->next;

	COMPILEUNIT* head = (COMPILEUNIT*)malloc(sizeof(COMPILEUNIT));

	head->file = files;
	head->tokens = tokenize(files->fullname);

	COMPILEUNIT* currunit = head;
	while(curr != NULL) {
		COMPILEUNIT* newunit = (COMPILEUNIT*)malloc(sizeof(COMPILEUNIT));
		newunit->file = curr;
		newunit->tokens = tokenize(curr->fullname);
		currunit->next = newunit;
		currunit = newunit;
		curr = curr->next;
	}
	currunit->next = NULL;

	actonunits(head, parseunit);

	CLASS* headclass = head->parsed;
	CLASS* currclass = headclass;
	currunit = head->next;
	while(currunit != NULL) {
		currclass->next = currunit->parsed;
		currclass = currunit->parsed;
		currunit = currunit->next;
	}
	currclass->next = NULL;
	COMPILER* compiler = mkcompiler(headclass);

	currunit = head;
	while(currunit != NULL) {
		currunit->compiler = compiler;
		currunit = currunit->next;
	}

	actonunits(head, compileunit);

	printlns(head->compiled->head, stdout);

	return 0;
}
