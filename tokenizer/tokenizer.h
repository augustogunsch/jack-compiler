#ifndef TOKENIZER_H
#define TOKENIZER_H
#include <stdio.h>

/* tokenizer
 * Simple tool that splits a stream into many tokens. */

typedef enum {
	keyword, identifier, symbol, integer, string
} TOKENTYPE;

typedef struct token {
	char* token;
	TOKENTYPE type;
	int definedat;
	struct token* next;
} TOKEN;

TOKEN* tokenize(char* filename);
void freetokens(TOKEN* t);
#endif
