#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "compiler.h"

void printcompiler(COMPILER* c) {
	printlns(c->output->head, stdout);
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

	PARSER* p = mkparser(tokenize(input), argv[1]);
	parse(p);
	COMPILER* c = mkcompiler(p->output);
	compile(c);
	printcompiler(c);
	
	return 0;
}
