#ifndef COMPILER_SCOPES_H
#define COMPILER_SCOPES_H
#include "parser-tree.h"

/* compiler-scopes
 * Tools for dealing with scopes.
 *
 * They can be used to create, expand and stack scopes, as well as to enforce
 * certain semantic rules. */

// Data types
typedef enum {
	subroutdec, classvardec, vardec, class, parameter
} OBJTYPE;

typedef struct object {
	void* pointer;
	DEBUGINFO* debug;
	STRINGLIST* names;
	OBJTYPE type;
	struct object* next;
} OBJ;

typedef struct scope {
	OBJ* objects;
	int condlabelcount;
	struct scope* previous;
} SCOPE;

// Group adding
void addclassvardecs(SCOPE* s, CLASSVARDEC* vs);
void addvardecs(SCOPE* s, VARDEC* vs);
void addsubroutdecs(SCOPE* s, SUBROUTDEC* ss);
void addclasses(SCOPE* s, CLASS* c);
void addparameters(SCOPE* s, PARAMETER* p);

// Scope handling
SCOPE* mkscope(SCOPE* prev);

// Single type getters
SUBROUTDEC* getsubroutdec(SCOPE* s, const char* name);
SUBROUTDEC* getsubroutdecfromcall(SCOPE* s, SUBROUTCALL* call);
CLASS* getclass(SCOPE* s, const char* name);

// Generic getters
OBJ* getbyname(SCOPE* s, const char* name);
#endif
