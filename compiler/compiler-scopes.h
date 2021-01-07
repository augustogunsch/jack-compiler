#ifndef COMPILER_SCOPES_H
#define COMPILER_SCOPES_H
#include "parser-tree.h"
#include "compiler.h"

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
	struct compiler* compiler;
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

struct compiler;

// Group adding
void addclassvardecs(SCOPE* s, CLASSVARDEC* classvardecs);
void addlocalvars(SCOPE* s, VARDEC* localvars);
void addparameters(SCOPE* s, bool isformethod, PARAMETER* params);

// Scope handling
SCOPE* mkscope(SCOPE* prev);

// Single type getters
SUBROUTDEC* getsubroutdecfromcall(SCOPE* s, SUBROUTCALL* call, VAR** varret);
CLASS* getclass(SCOPE* s, const char* name);

// Generic getters
VAR* getvar(SCOPE* s, const char* name);
VAR* getvarmustexist(SCOPE* s, DEBUGINFO* d, const char* name);

// Freeing
void freescope(SCOPE* s);
#endif
