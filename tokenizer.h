#ifndef TOKENIZER_H
#define TOKENIZER_H
#include <stdio.h>

typedef enum {
	keyword, identifier, symbol, integer, string
} TOKENTYPE;

typedef struct token {
	char* token;
	TOKENTYPE type;
	int truen;
	struct token* next;
} TOKEN;

TOKEN* tokenize(FILE* input);
void freetokenlist(TOKEN l);
#endif
