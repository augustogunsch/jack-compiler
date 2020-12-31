#include <stdlib.h>
#include "compiler-structure.h"
#include "compiler.h"

/* This should be part of compiler-structure, but since it is used by other modules,
 * it will stay here for convenience */
LINEBLOCK* compileclass(COMPILER* c, CLASS* class) {
	SCOPE* topscope = mkscope(c->globalscope);
	if(class->vardecs != NULL)
		addclassvardecs(c, topscope, class->vardecs);
	if(class->subroutdecs != NULL)
		topscope->subroutines = class->subroutdecs;

	LINEBLOCK* output = NULL;
	SUBROUTDEC* curr = class->subroutdecs;
	while(curr != NULL) {
		output = mergelnblks(output, compilesubroutdec(c, topscope, class, curr));
		curr = curr->next;
	}
	return output;
}

COMPILER* mkcompiler(CLASS* classes) {
	COMPILER* c = (COMPILER*)malloc(sizeof(COMPILER));
	c->globalscope = mkscope(NULL);
	c->globalscope->classes = classes;
	c->classes = classes;
	pthread_mutex_init(&(c->ifmutex), NULL);
	pthread_mutex_init(&(c->whilemutex), NULL);
	pthread_mutex_init(&(c->staticmutex), NULL);
	return c;
}

void freecompiler(COMPILER* c) {
	pthread_mutex_destroy(&(c->ifmutex));
	pthread_mutex_destroy(&(c->whilemutex));
	pthread_mutex_destroy(&(c->staticmutex));
	// to be continued
	free(c);
}
