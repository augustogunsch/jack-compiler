#ifndef THREADS_H
#define THREADS_H
#include <pthread.h>
#include "parser.h"
#include "compiler.h"
#include "io.h"

/* threads
 * Tools for dealing with the compiling pipeline in a parallel way */

typedef struct unit {
	FILELIST* file;
	TOKEN* tokens;
	CLASS* parsed;
	COMPILER* compiler;
	LINEBLOCK* compiled;
	struct unit* next;
} COMPILEUNIT;

void* parseunit(void* input);
void* compileunit(void* input);
void waitthreads(pthread_t* threads, int amount);
void actonunits(COMPILEUNIT* units, void*(*fun)(void*));
#endif
