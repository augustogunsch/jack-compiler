#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parser.h"
#include "parser-structure.h"

// Statements
PARSER* mkparser(TOKEN* tokens, char* file) {
	PARSER* parser = (PARSER*)malloc(sizeof(PARSER));
	parser->tokens = tokens;
	parser->current = tokens;
	parser->file = file;
	return parser;
}

void parse(PARSER* parser) {
	parser->output = parseclasses(parser);
}
