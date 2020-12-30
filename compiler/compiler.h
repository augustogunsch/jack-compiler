#ifndef COMPILER_H
#define COMPILER_H
#include "compiler-scopes.h"
#include "parser-tree.h"
#include "vm-lines.h"

typedef struct {
	CLASS* classes;
	SCOPE* globalscope;
} COMPILER;

COMPILER* mkcompiler(CLASS* classes);
LINEBLOCK* compileclass(COMPILER* c, CLASS* class);
#endif
