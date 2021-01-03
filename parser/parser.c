#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parser.h"
#include "parser-structure.h"

PARSER* mkparser(TOKEN* t, char* file) {
	PARSER* parser = (PARSER*)malloc(sizeof(PARSER));
	parser->tokens = t;
	parser->current = t;
	parser->file = file;
	return parser;
}

CLASS* parse(PARSER* p) {
	return parseclass(p);
}

void freeparser(PARSER* p) {
	freetokens(p->tokens);
	free(p);
}
