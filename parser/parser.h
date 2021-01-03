#ifndef PARSER_H
#define PARSER_H
#include "tokenizer.h"
#include "parser-tree.h"

/* parser
 * This is the file that should be included in other modules
 * that want to parse a file. */

typedef struct {
	TOKEN* tokens;
	TOKEN* current;
	TOKEN* checkpoint;
	char* file;
} PARSER;

PARSER* mkparser(TOKEN* t, char* file);
CLASS* parse(PARSER* p);
void freeparser(PARSER* p);
#endif
