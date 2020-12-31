#include <stdlib.h>
#include <string.h>
#include "compiler-expressions.h"
#include "compiler-statements.h"
#include "compiler-util.h"

/* BEGIN FORWARD DECLARATIONS */

// Miscelaneous
LINE* popthat();
LINE* pushtemp();
char* mkcondlabel(char* name, int count);

// Handling individual statements
LINEBLOCK* compileret(SCOPE* s, TERM* e);
LINEBLOCK* compileif(COMPILER* c, SCOPE* s, IFSTATEMENT* st);
LINEBLOCK* compilewhile(COMPILER* c, SCOPE* s, CONDSTATEMENT* w);
LINEBLOCK* compilelet(SCOPE* s, LETSTATEMENT* l);
LINEBLOCK* compilestatement(COMPILER* c, SCOPE* s, STATEMENT* st);

/* END FORWARD DECLARATIONS */



// Miscelaneous
LINE* popthat() {
	char* popthat[] = { "pop", "that", "0" };
	return mkln(popthat);
}

LINE* pushtemp() {
	char* pushtemp[] = { "push", "temp", "0" };
	return mkln(pushtemp);
}

char* mkcondlabel(char* name, int count) {
	int sz = (strlen(name) + countplaces(count) + 1) * sizeof(char);
	char* result = (char*)malloc(sz);
	sprintf(result, "%s%i", name, count);
	return result;
}

// Handling individual statements
LINEBLOCK* compileret(SCOPE* s, TERM* e) {
	LINE* ret = onetoken("return");
	LINEBLOCK* blk = mklnblk(ret);

	// void subroutdecs return 0
	if(e == NULL) {
		char* tokens[] = { "push", "constant", "0" };
		appendlnbefore(blk, mkln(tokens));
	} else
		blk = mergelnblks(compileexpression(s, e), blk);

	return blk;
}

LINEBLOCK* compileif(COMPILER* c, SCOPE* s, IFSTATEMENT* st) {
	LINEBLOCK* blk = compileexpression(s, st->base->expression);

	pthread_mutex_lock(&(c->ifmutex));
	static int ifcount = 0;
	int mycount = ifcount;
	ifcount++;
	pthread_mutex_unlock(&(c->ifmutex));
	
	char* truelabel = mkcondlabel("IF_TRUE", mycount);
	char* ifgoto[] = { "if-goto", truelabel };
	appendln(blk, mkln(ifgoto));
	
	char* falselabel = mkcondlabel("IF_FALSE", mycount);
	char* gotofalse[] = { "goto", falselabel };
	appendln(blk, mkln(gotofalse));

	char* truelabelln[] = { "label", truelabel };
	appendln(blk, mkln(truelabelln));

	blk = mergelnblks(blk, compilestatements(c, s, st->base->statements));

	char* endlabel;
	bool haselse = st->elsestatements != NULL;
	if(haselse) {
		endlabel = mkcondlabel("IF_END", mycount);
		char* endgoto[] = { "goto", endlabel };
		appendln(blk, mkln(endgoto));
	}

	char* falselabelln[] = { "label", falselabel};
	appendln(blk, mkln(falselabelln));

	if(haselse) {
		blk = mergelnblks(blk, compilestatements(c, s, st->elsestatements));
		char* endlabelln[] = { "label", endlabel };
		appendln(blk, mkln(endlabelln));
		free(endlabel);
	}

	free(falselabel);
	free(truelabel);

	return blk;
}

LINEBLOCK* compilewhile(COMPILER* c, SCOPE* s, CONDSTATEMENT* w) {
	LINEBLOCK* blk = compileexpression(s, w->expression);

	pthread_mutex_lock(&(c->whilemutex));
	static int whilecount = 0;
	int mycount = whilecount;
	whilecount++;
	pthread_mutex_unlock(&(c->whilemutex));

	char* explabel = mkcondlabel("WHILE_EXP", mycount);
	char* explabelln[] = { "label", explabel };
	appendlnbefore(blk, mkln(explabelln));

	appendln(blk, onetoken("not"));

	char* endlabel = mkcondlabel("WHILE_END", mycount);
	char* ifgoto[] = { "if-goto", endlabel };
	appendln(blk, mkln(ifgoto));

	blk = mergelnblks(blk, compilestatements(c, s, w->statements));

	char* gotoln[] = { "goto", explabel };
	appendln(blk, mkln(gotoln));

	char* endlabelln[] = { "label", endlabel };
	appendln(blk, mkln(endlabelln));

	free(explabel);
	free(endlabel);

	return blk;
}

LINEBLOCK* compilelet(SCOPE* s, LETSTATEMENT* l) {
	LINEBLOCK* blk = compileexpression(s, l->expression);

	if(l->arrayind != NULL) {
		appendlnbefore(blk, onetoken("add"));
		appendlnbefore(blk, pushvar(s, l->varname));
		blk = mergelnblks(compileexpression(s, l->arrayind), blk);

		appendln(blk, poptemp());
		appendln(blk, popthatadd());
		appendln(blk, pushtemp());
		appendln(blk, popthat());
	}
	else
		appendln(blk, popvar(s, l->varname));
	return blk;
}

LINEBLOCK* compilestatement(COMPILER* c, SCOPE* s, STATEMENT* st) {
	s->currdebug = st->debug;
	if(st->type == dostatement) return compilesubroutcall(s, st->dostatement);
	if(st->type == returnstatement) return compileret(s, st->retstatement);
	if(st->type == ifstatement) return compileif(c, s, st->ifstatement);
	if(st->type == whilestatement) return compilewhile(c, s, st->whilestatement);
	if(st->type == letstatement) return compilelet(s, st->letstatement);
	eprintf("UNSUPPORTED type %i\n", st->type);
	exit(1);
}

LINEBLOCK* compilestatements(COMPILER* c, SCOPE* s, STATEMENT* sts) {
	LINEBLOCK* head = NULL;
	while(sts != NULL) {
		head = mergelnblks(head, compilestatement(c, s, sts));
		sts = sts->next;
	}
	return head;
}
