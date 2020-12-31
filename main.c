#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "threads.h"
#include "parser.h"
#include "compiler.h"
#include "io.h"
#include "os.h"

int main(int argc, char* argv[]) {
	if(argc < 2) {
		eprintf("Usage: %s {input file(s)}\n", argv[0]);
		return 1;
	}

	FILELIST* files = getfiles(argv[1]);
	FILELIST* curr = files->next;

	COMPILEUNIT* head = (COMPILEUNIT*)malloc(sizeof(COMPILEUNIT));

	head->file = files;
	head->tokens = tokenize(files->fullname);

	COMPILEUNIT* currunit = head;
	while(curr != NULL) {
		COMPILEUNIT* newunit = (COMPILEUNIT*)malloc(sizeof(COMPILEUNIT));
		newunit->file = curr;
		newunit->tokens = tokenize(curr->fullname);
		currunit->next = newunit;
		currunit = newunit;
		curr = curr->next;
	}
	currunit->next = NULL;

	actonunits(head, parseunit);

	CLASS* headclass = head->parsed;
	CLASS* currclass = headclass;
	currunit = head->next;
	while(currunit != NULL) {
		currclass->next = currunit->parsed;
		currclass = currunit->parsed;
		currunit = currunit->next;
	}
	currclass->next = NULL;
	COMPILER* compiler = mkcompiler(headclass);

	currunit = head;
	while(currunit != NULL) {
		currunit->compiler = compiler;
		currunit = currunit->next;
	}

	populateos();
	actonunits(head, compileunit);

	currunit = head;
	while(currunit != NULL) {
		FILE* output = fopen(currunit->file->outname, "w");
		if(output == NULL) {
			eprintf("%s", strerror(errno));
			exit(1);
		}
		printlns(currunit->compiled->head, output);
		fclose(output);
		currunit = currunit->next;
	}

	//freeos();
	return 0;
}
