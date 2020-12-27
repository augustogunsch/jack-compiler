#ifndef COMPILER_SCOPES_H
#define COMPILER_SCOPES_H
#include "parser-tree.h"

/* compiler-scopes
 * Tools for dealing with scopes.
 *
 * They can be used to create, expand and stack scopes, as well as to enforce
 * certain semantic rules. */

// Data types
typedef struct var {
	DEBUGINFO* debug;
	char* memsegment;
	char* type;
	char* name;
	int index;
	bool primitive;
	struct var* next;
} VAR;

typedef struct scope {
	DEBUGINFO* currdebug;
	CLASS* currclass;

	CLASS* classes;
	SUBROUTDEC* subroutines;

	VAR* fields;
	VAR* staticvars;
	VAR* localvars;
	VAR* parameters;

	struct scope* previous;
} SCOPE;

// Group adding
void addclassvardecs(SCOPE* s, CLASSVARDEC* classvardecs);
void addlocalvars(SCOPE* s, VARDEC* localvars);
void addparameters(SCOPE* s, PARAMETER* params);

// Scope handling
SCOPE* mkscope(SCOPE* prev);

// Single type getters
SUBROUTDEC* getsubroutdecfromcall(SCOPE* s, SUBROUTCALL* call);
CLASS* getclass(SCOPE* s, const char* name);

// Generic getters
VAR* getvar(SCOPE* s, const char* name);
#endif
