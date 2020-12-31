#ifndef COMPILER_UTIL_H
#define COMPILER_UTIL_H
#include "vm-lines.h"
#include "compiler-scopes.h"
#include "compiler.h"

/* compiler-util
 * Random utilities for the compiler. */

#define mkln(id) mksimpleln(id, strcount(id))

LINE* onetoken(char* str);
LINE* mksimpleln(char** tokens, int count);

LINE* pushvarraw(SCOPE*s, VAR* v);
LINE* pushvar(SCOPE* s, const char* name);
LINE* popvar(SCOPE* s, const char* name);
LINE* poptemp();
LINE* popthatadd();

#endif
