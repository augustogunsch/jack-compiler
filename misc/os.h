#ifndef OS_H
#define OS_H
#include "parser-tree.h"

SUBROUTDEC* getossubroutdec(CLASS* os, SUBROUTCALL* call);
CLASS* getosclass(CLASS* os, const char* name);
CLASS* mkos();
void freeos(CLASS* os);

#endif
