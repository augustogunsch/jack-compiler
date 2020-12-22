#ifndef PARSER_H
#define PARSER_H
#include "tokenizer.h"
#include "parser-tree.h"

typedef struct {
	TOKEN* tokens;
	TOKEN* current;
	TOKEN* checkpoint;
	char* file;
	CLASS* output;
} PARSER;

PARSER* mkparser(TOKEN* tokens, char* file);
void parse(PARSER* p);
#endif
