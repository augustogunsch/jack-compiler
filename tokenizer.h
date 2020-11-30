#ifndef TOKENIZER_H
#define TOKENIZER_H
#include <stdio.h>

typedef enum {
	keyword, symbol, integer, string, identifier
} TOKENTYPE;

typedef struct tklist {
	char* token;
	TOKENTYPE type;
	int truen;
	struct tklist* next;
} TOKENLIST;

TOKENLIST* tokenize(FILE* input);
void freetokenlist(TOKENLIST l);
#endif
