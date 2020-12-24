#ifndef COMPILER_H
#define COMPILER_H
#include "compiler-scopes.h"
#include "parser-tree.h"
#include "vm-lines.h"

typedef struct {
	CLASS* classes;
	SCOPE* globalscope;
	LINEBLOCK* output;
} COMPILER;

COMPILER* mkcompiler(CLASS* classes);
void compile(COMPILER* c);

#endif
