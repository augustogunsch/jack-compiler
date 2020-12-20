#ifndef COMPILER_H
#define COMPILER_H
#include "util.h"
#include "parser.h"

typedef struct scope {
	SUBDEC* subroutines;
	CLASSVARDEC* classvardecs;
	VARDEC* vardecs;
	CLASS* classes;
	struct scope* previous;
} SCOPE;

typedef struct {
	SCOPE* globalscope;
	LINE* output;
	LINE* lastln;
} COMPILER;

COMPILER* mkcompiler(CLASS* classes);
void compile();

#endif
