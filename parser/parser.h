#ifndef PARSER_H
#define PARSER_H
#include "tokenizer.h"
#include "parser-tree.h"

typedef struct {
	TOKEN* tokens;
	TOKEN* current;
	TOKEN* checkpoint;
	char* file;
} PARSER;

CLASS* parse(TOKEN* tokens, char* file);
#endif
