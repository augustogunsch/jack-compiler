#ifndef OS_H
#define OS_H
#include "parser-tree.h"

SUBROUTDEC* getossubroutdec(SUBROUTCALL* call);
CLASS* getosclass(const char* name);
void populateos();
void freeos();

#endif
