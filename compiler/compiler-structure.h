#ifndef COMPILER_STRUCTURE_H
#define COMPILER_STRUCTURE_H
#include "compiler.h"

/* compiler-structure
 * Module for dealing with and compiling general program structure. */

LINEBLOCK* compilesubroutdec(COMPILER* c, SCOPE* s, CLASS* cl, SUBROUTDEC* sd);
#endif
