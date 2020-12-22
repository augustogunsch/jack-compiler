#ifndef PARSER_CONSTANTS_H
#define PARSER_CONSTANTS_H
#include "util.h"

const char* keywordsarr[] = { "true", "false", "null", "this" };
const char* opsarr[] = { "+", "-", "*", "/", "&", "|", "<", ">", "=" };
const char* classvartypesarr[] = { "static", "field" };
const char* vartypesarr[] = { "int", "char", "boolean" };
const char* subroutclassesarr[] = { "constructor", "function", "method" };
const char* tokentypesarr[] = { "keyword", "identifier", "symbol", "integerConstant", "stringConstant" };

#define mkstrlist(name, array) STRINGARRAY name = { .items = array, .size = strcount(array) }
mkstrlist(keywordconstants, keywordsarr);
mkstrlist(operators, opsarr);
mkstrlist(classvartypes, classvartypesarr);
mkstrlist(vartypes, vartypesarr);
mkstrlist(subroutclasses, subroutclassesarr);
mkstrlist(tokentypes, tokentypesarr);
#undef mkstrlist
#endif
