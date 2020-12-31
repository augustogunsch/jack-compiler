#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "compiler-util.h"
#include "compiler-expressions.h"

LINEBLOCK* compilestatements(COMPILER* c, SCOPE* s, STATEMENT* sts);
LINEBLOCK* compileexpression(SCOPE* s, TERM* e);

int countparameters(PARAMETER* params) {
	int i = 0;
	while(params != NULL) {
		i++;
		params = params->next;
	}
	return i;
}

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

char* mkcondlabel(char* name, int count) {
	int sz = (strlen(name) + countplaces(count) + 1) * sizeof(char);
	char* result = (char*)malloc(sz);
	sprintf(result, "%s%i", name, count);
	return result;
}

LINE* pushthatadd() {
	char* pushthatadd[] = { "push", "pointer", "1" };
	return mkln(pushthatadd);
}

LINE* popthat() {
	char* popthat[] = { "pop", "that", "0" };
	return mkln(popthat);
}

LINE* pushtemp() {
	char* pushtemp[] = { "push", "temp", "0" };
	return mkln(pushtemp);
}

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
	}

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

LINEBLOCK* compilefunbody(COMPILER* c, SCOPE* s, CLASS* cl, SUBROUTBODY* b) {
	SCOPE* myscope = mkscope(s);
	myscope->currclass = cl;
	if(b->vardecs != NULL)
		addlocalvars(s, b->vardecs);
	LINEBLOCK* head = compilestatements(c, myscope, b->statements);
	return head;
}

LINE* mksubdeclabel(CLASS* c, SUBROUTDEC* sd) {
	char* labelstrs[] = { "function", dotlabel(c->name, sd->name), itoa(countlocalvars(sd->body->vardecs)) };
	LINE* label = mkln(labelstrs);
	free(labelstrs[1]);
	free(labelstrs[2]);
	label->next = NULL;
	return label;
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
	// 'this' and arguments are pushed by caller
	// Must have a 'return' at the end
	// Label names must have class name too (see mapping)
	
	// types: method, function, constructor
	// must switch all of these
	SCOPE* myscope = mkscope(s);
	if(sd->parameters != NULL)
		addparameters(myscope, sd->subroutclass == method, sd->parameters);
	if(sd->subroutclass == function)
		return compilefundec(c, myscope, cl, sd);
	if(sd->subroutclass == constructor)
		return compileconstructor(c, myscope, cl, sd);
	return compilemethod(c, myscope, cl, sd);
}

LINEBLOCK* compileclass(COMPILER* c, CLASS* class) {
	SCOPE* topscope = mkscope(c->globalscope);
	if(class->vardecs != NULL)
		addclassvardecs(c, topscope, class->vardecs);
	if(class->subroutdecs != NULL)
		topscope->subroutines = class->subroutdecs;

	LINEBLOCK* output = NULL;
	SUBROUTDEC* curr = class->subroutdecs;
	while(curr != NULL) {
		output = mergelnblks(output, compilesubroutdec(c, topscope, class, curr));
		curr = curr->next;
	}
	return output;
}

COMPILER* mkcompiler(CLASS* classes) {
	COMPILER* c = (COMPILER*)malloc(sizeof(COMPILER));
	c->globalscope = mkscope(NULL);
	c->globalscope->classes = classes;
	c->classes = classes;
	pthread_mutex_init(&(c->ifmutex), NULL);
	pthread_mutex_init(&(c->whilemutex), NULL);
	pthread_mutex_init(&(c->staticmutex), NULL);
	return c;
}

void freecompiler(COMPILER* c) {
	pthread_mutex_destroy(&(c->ifmutex));
	pthread_mutex_destroy(&(c->whilemutex));
	pthread_mutex_destroy(&(c->staticmutex));
	// to be continued
	free(c);
}
