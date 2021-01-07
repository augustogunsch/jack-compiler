#ifndef COMPILER_STATEMENTS_H
#define COMPILER_STATEMENTS_H
#include "compiler.h"

/* compiler-statements
 * Single function for compiling statements */

LINEBLOCK* compilestatements(SCOPE* s, STATEMENT* sts);
LINEBLOCK* compilestatementsretlast(SCOPE* s, STATEMENT* sts, STATEMENT** retlast);
#endif
