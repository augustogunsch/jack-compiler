#ifndef COMPILER_EXPRESSIONS_H
#define COMPILER_EXPRESSIONS_H
#include "vm-lines.h"
#include "compiler.h"

/* compiler-expressions
 * Subroutines for dealing and compiling expressions and singular terms. */

// Dealing with singular terms
LINEBLOCK* compilesubroutcall(SCOPE* s, SUBROUTCALL* call);

// Dealing with whole expressions
LINEBLOCK* compileexpression(SCOPE* s, TERM* e);
LINEBLOCK* compileexplist(SCOPE* s, EXPRESSIONLIST* explist);
#endif
