#ifndef COMPILER_H
#define COMPILER_H
#include "util.h"
#include "parser.h"
#include "vm-lines.h"
#include "compiler-scopes.h"

typedef struct {
	SCOPE* globalscope;
	LINEBLOCK* output;
} COMPILER;

COMPILER* mkcompiler(CLASS* classes);
void compile();

#endif
