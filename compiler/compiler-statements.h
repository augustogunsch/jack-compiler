#ifndef COMPILER_STATEMENTS_H
#define COMPILER_STATEMENTS_H
#include "compiler.h"

/* compiler-statements
 * Single function for compiling statements */

LINEBLOCK* compilestatements(COMPILER* c, SCOPE* s, STATEMENT* sts);
#endif
