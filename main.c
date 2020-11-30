#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "tokenizer.h"

const char* types[] = {
	"keyword", "symbol", "integerConstant", "stringConstant", "identifier"
};

void printtks(TOKENLIST* tks, FILE* output) {
	fprintf(output, "<%s> %s </%s>\r\n", types[tks->type], tks->token, types[tks->type]);
	TOKENLIST* next = tks->next;
	free(tks->token);
	free(tks);
	if(next != NULL)
		printtks(next, output);
}

int main(int argc, char* argv[]) {
	if(argc < 2) {
		fprintf(stderr, "Usage: %s {input file}\n", argv[0]);
		return 1;
	}

	FILE* input = fopen(argv[1], "r");

	if(input == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		return errno;
	}

	FILE* output = fopen("out.xml", "w");
	fprintf(output, "<tokens>\r\n");
	printtks(tokenize(input), output);
	fprintf(output, "</tokens>\r\n");
	fclose(output);
	
	return 0;
}
