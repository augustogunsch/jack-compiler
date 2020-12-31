#ifndef COMPILER_H
#define COMPILER_H
#include "compiler-scopes.h"
#include "parser-tree.h"
#include "vm-lines.h"

typedef struct {
	pthread_mutex_t ifmutex;
	pthread_mutex_t whilemutex;
	CLASS* classes;
	SCOPE* globalscope;
} COMPILER;

COMPILER* mkcompiler(CLASS* classes);
LINEBLOCK* compileclass(COMPILER* c, CLASS* class);
void freecompiler(COMPILER* c);
#endif
