#ifndef COMPILER_EXPRESSIONS_H
#define COMPILER_EXPRESSIONS_H
#include "vm-lines.h"
#include "compiler.h"

/* compiler-expressions
 * Functions for dealing and compiling expressions and singular terms. */

// Dealing with singular terms
LINEBLOCK* compilesubroutcall(SCOPE* s, SUBROUTCALL* call);

// Dealing with whole expressions
LINEBLOCK* compileexpression(SCOPE* s, DEBUGINFO* d, TERM* e);
LINEBLOCK* compileexplist(SCOPE* s, DEBUGINFO* d, EXPRESSIONLIST* explist);
#endif
