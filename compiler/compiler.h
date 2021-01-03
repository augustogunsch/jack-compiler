#ifndef COMPILER_H
#define COMPILER_H
#include <pthread.h>
#include "parser-tree.h"
#include "vm-lines.h"
#include "compiler-scopes.h"

/* compiler
 * This is the file that should be included in other modules
 * that want to compile a class/program. */

struct scope;
typedef struct compiler {
	pthread_mutex_t ifmutex;
	pthread_mutex_t whilemutex;
	pthread_mutex_t staticmutex;
	CLASS* classes;
	CLASS* os;
	struct scope* globalscope;
} COMPILER;

COMPILER* mkcompiler(CLASS* classes);
LINEBLOCK* compileclass(COMPILER* c, CLASS* class);
void freecompiler(COMPILER* c);
#endif
