#ifndef PARSER_EXPRESSIONS_H
#define PARSER_EXPRESSIONS_H
#include "parser.h"
TERM* parseexpressionnullified(PARSER* p);
TERM* parseexpression(PARSER* p);
SUBROUTCALL* parsesubroutcall(PARSER* p);
EXPRESSIONLIST* parseexpressionlist(PARSER* p);
#endif
