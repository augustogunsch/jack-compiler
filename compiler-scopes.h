#ifndef COMPILER_SCOPES_H
#define COMPILER_SCOPES_H
#include "parser.h"

/* compiler-scopes
 * Tools for dealing with scopes.
 *
 * They can be used to create, expand and stack scopes, as well as to enforce
 * certain semantic rules. */

// Data types
typedef struct scope {
	SUBDEC* subroutines;
	CLASSVARDEC* classvardecs;
	VARDEC* vardecs;
	CLASS* classes;
	struct scope* previous;
} SCOPE;

typedef enum {
	subdec, classvardec, vardec, class
} OBJTYPE;

typedef struct object {
	OBJTYPE type;
	union {
		SUBDEC* subdec;
		CLASSVARDEC* classvardec;
		VARDEC* vardec;
		CLASS* class;
	};
	DEBUGINFO* (*getdebug)(struct object*);
} OBJ;

// Group adding
void addclassvardecs(SCOPE* s, CLASSVARDEC* vs);
void addvardecs(SCOPE* s, VARDEC* vs);
void addsubdecs(SCOPE* s, SUBDEC* ss);
void addclasses(SCOPE* s, CLASS* c);

// Scope handling
SCOPE* mkscope(SCOPE* prev);

// Single type getters
SUBDEC* getsubdec(SCOPE* s, char* name);
SUBDEC* getsubdecfromcall(SCOPE* s, SUBROUTCALL* call);
CLASS* getclass(SCOPE* s, char* name);

// Generic getters
OBJ* getbyname(SCOPE* s, char* name);
#endif
