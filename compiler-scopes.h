#ifndef COMPILER_SCOPES_H
#define COMPILER_SCOPES_H
#include "parser-tree.h"

/* compiler-scopes
 * Tools for dealing with scopes.
 *
 * They can be used to create, expand and stack scopes, as well as to enforce
 * certain semantic rules. */

// Data types
typedef struct scope {
	SUBROUTDEC* subroutines;
	CLASSVARDEC* classvardecs;
	VARDEC* vardecs;
	CLASS* classes;
	int condlabelcount;
	struct scope* previous;
} SCOPE;

typedef enum {
	subroutdec, classvardec, vardec, class
} OBJTYPE;

typedef struct object {
	OBJTYPE type;
	union {
		SUBROUTDEC* subroutdec;
		CLASSVARDEC* classvardec;
		VARDEC* vardec;
		CLASS* class;
	};
	DEBUGINFO* (*getdebug)(struct object*);
} OBJ;

// Group adding
void addclassvardecs(SCOPE* s, CLASSVARDEC* vs);
void addvardecs(SCOPE* s, VARDEC* vs);
void addsubroutdecs(SCOPE* s, SUBROUTDEC* ss);
void addclasses(SCOPE* s, CLASS* c);

// Scope handling
SCOPE* mkscope(SCOPE* prev);

// Single type getters
SUBROUTDEC* getsubroutdec(SCOPE* s, const char* name);
SUBROUTDEC* getsubroutdecfromcall(SCOPE* s, SUBROUTCALL* call);
CLASS* getclass(SCOPE* s, const char* name);

// Generic getters
OBJ* getbyname(SCOPE* s, const char* name);
#endif
