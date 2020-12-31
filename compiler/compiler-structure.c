#include <stdlib.h>
#include "compiler-statements.h"
#include "compiler-structure.h"
#include "compiler-util.h"

/* BEGIN FORWARD DECLARATIONS */

// Miscelaneous
int countlocalvars(VARDEC* decs);
int countstrs(STRINGLIST* ls);
int getobjsize(CLASS* c);
LINE* mksubdeclabel(CLASS* c, SUBROUTDEC* sd);

// Compiling methods
LINEBLOCK* compilefunbody(COMPILER* c, SCOPE* s, CLASS* cl, SUBROUTBODY* b);
LINEBLOCK* compilefundec(COMPILER* c, SCOPE* s, CLASS* cl, SUBROUTDEC* f);
LINEBLOCK* compileconstructor(COMPILER* c, SCOPE* s, CLASS* cl, SUBROUTDEC* con);
LINEBLOCK* compilemethod(COMPILER* c, SCOPE* s, CLASS* cl, SUBROUTDEC* m);

/* END FORWARD DECLARATIONS */


// Miscelaneous
int countlocalvars(VARDEC* decs) {
	int i = 0;
	while(decs != NULL) {
		STRINGLIST* curr = decs->names;
		while(curr != NULL) {
			i++;
			curr = curr->next;
		}
		decs = decs->next;
	}
	return i;
}

int countstrs(STRINGLIST* ls) {
	int count = 0;
	while(ls != NULL) {
		count++;
		ls = ls->next;
	}
	return count;
}

int getobjsize(CLASS* c) {
	CLASSVARDEC* curr = c->vardecs;
	int count = 0;
	while(curr != NULL) {
		if(curr->type == field)
			count += countstrs(curr->base->names);
		curr = curr->next;
	}
	return count;
}

LINE* mksubdeclabel(CLASS* c, SUBROUTDEC* sd) {
	char* labelstrs[] = { "function", dotlabel(c->name, sd->name), itoa(countlocalvars(sd->body->vardecs)) };
	LINE* label = mkln(labelstrs);
	free(labelstrs[1]);
	free(labelstrs[2]);
	label->next = NULL;
	return label;
}

// Compiling methods
LINEBLOCK* compilefunbody(COMPILER* c, SCOPE* s, CLASS* cl, SUBROUTBODY* b) {
	SCOPE* myscope = mkscope(s);
	myscope->currclass = cl;
	if(b->vardecs != NULL)
		addlocalvars(s, b->vardecs);
	LINEBLOCK* head = compilestatements(c, myscope, b->statements);
	return head;
}

LINEBLOCK* compilefundec(COMPILER* c, SCOPE* s, CLASS* cl, SUBROUTDEC* f) {
	LINE* label = mksubdeclabel(cl, f);

	if(f->body->statements != NULL) {
		LINEBLOCK* body = compilefunbody(c, s, cl, f->body);
		appendlnbefore(body, label);
		return body;
	}
	else
		return mklnblk(label);
}

LINEBLOCK* compileconstructor(COMPILER* c, SCOPE* s, CLASS* cl, SUBROUTDEC* con) {
	LINE* label = mksubdeclabel(cl, con);
	LINEBLOCK* blk = mklnblk(label);

	char* size[] = { "push", "constant", itoa(getobjsize(cl)) };
	char* memalloc[] = { "call", "Memory.alloc", "1" };
	char* poppointer[] = { "pop", "pointer", "0" };
	appendln(blk, mkln(size));
	appendln(blk, mkln(memalloc));
	appendln(blk, mkln(poppointer));
	free(size[2]);

	if(con->body != NULL)
		return mergelnblks(blk, compilefunbody(c, s, cl, con->body));
	else
		return blk;
}

LINEBLOCK* compilemethod(COMPILER* c, SCOPE* s, CLASS* cl, SUBROUTDEC* m) {
	LINE* label = mksubdeclabel(cl, m);
	LINEBLOCK* blk = mklnblk(label);

	char* pusharg0[] = { "push", "argument", "0" };
	char* poppointer[] = { "pop", "pointer", "0" };
	appendln(blk, mkln(pusharg0));
	appendln(blk, mkln(poppointer));

	if(m->body != NULL) 
		return mergelnblks(blk, compilefunbody(c, s, cl, m->body));
	else
		return blk;
}

LINEBLOCK* compilesubroutdec(COMPILER* c, SCOPE* s, CLASS* cl, SUBROUTDEC* sd) {
	SCOPE* myscope = mkscope(s);
	if(sd->parameters != NULL)
		addparameters(myscope, sd->subroutclass == method, sd->parameters);
	if(sd->subroutclass == function)
		return compilefundec(c, myscope, cl, sd);
	if(sd->subroutclass == constructor)
		return compileconstructor(c, myscope, cl, sd);
	return compilemethod(c, myscope, cl, sd);
}
