#ifndef OS_H
#define OS_H
#include "parser-tree.h"

SUBROUTDEC* getossubroutdec(SUBROUTCALL* call);
void populateos();
void freeos();

#endif
