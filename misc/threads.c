#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "threads.h"

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
			code = pthread_create(&mythreads[i], &attr, fun, curr);

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
