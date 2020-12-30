#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parser.h"
#include "parser-structure.h"

CLASS* parse(TOKEN* tokens, char* file) {
	PARSER* parser = (PARSER*)malloc(sizeof(PARSER));
	parser->tokens = tokens;
	parser->current = tokens;
	parser->file = file;
	return parseclass(parser);
}
