#include <stdlib.h>
#include <string.h>
#include "compiler.h"

LINEBLOCK* compilestatements(COMPILER* c, SCOPE* s, STATEMENT* sts);
LINEBLOCK* compilesubroutcall(SCOPE* s, SUBROUTCALL* call);
LINEBLOCK* compileexpression(SCOPE* s, TERM* e);

int countparameters(PARAMETER* params) {
	int i = 0;
	while(params != NULL) {
		i++;
		params = params->next;
	}
	return i;
}

int countexpressions(EXPRESSIONLIST* explist) {
	int i = 0;
	while(explist != NULL) {
		i++;
		explist = explist->next;
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

char* dotlabel(char* n1, char* n2) {
	int sz = (strlen(n1) + strlen(n2) + 2) * sizeof(char);
	char* result = (char*)malloc(sz);
	sprintf(result, "%s.%s", n1, n2);
	return result;
}

char* mkcondlabel(char* name, int count) {
	int sz = (strlen(name) + countplaces(count) + 1) * sizeof(char);
	char* result = (char*)malloc(sz);
	sprintf(result, "%s%i", name, count);
	return result;
}

LINE* onetoken(char* str) {
	LINE* ln = mkline(1);
	addtoken(ln, ezheapstr(str));
	ln->next = NULL;
	return ln;
}

LINE* mksimpleln(char** tokens, int count) {
	LINE* ln = mkline(count);
	for(int i = 0; i < count; i++)
		addtoken(ln, ezheapstr(tokens[i]));
	ln->next = NULL;
	return ln;
}

LINE* mathopln(char op) {
	if(op == '+')
		return onetoken("add");
	if(op == '-')
		return onetoken("sub");
	if(op == '=')
		return onetoken("eq");
	if(op == '>')
		return onetoken("gt");
	if(op == '<')
		return onetoken("lt");
	if(op == '|')
		return onetoken("or");
	if(op == '&')
		return onetoken("and");
	if(op == '/') {
		char* tokens[] = { "call", "Math.divide", "2" };
		return mksimpleln(tokens, strcount(tokens));
	}
	if(op == '*') {
		char* tokens[] = { "call", "Math.multiply", "2" };
		return mksimpleln(tokens, strcount(tokens));
	}
}

LINEBLOCK* pushconstant(int n) {
	char* tokens[] = { "push", "constant", itoa(n) };
	return mklnblk(mksimpleln(tokens, strcount(tokens)));
}

LINEBLOCK* pushunaryopterm(SCOPE* s, TERM* t) {
	LINEBLOCK* blk = compileexpression(s, t->expression);
	LINE* neg;
	if(t->unaryop == '-')
	       	neg = onetoken("neg");
	else
		neg = onetoken("not");
	appendln(blk, neg);
	return blk;
}

LINEBLOCK* opvarraw(SCOPE* s, char* op, VAR* v) {
	char* tokens[] = { op, v->memsegment, itoa(v->index) };
	return mklnblk(mksimpleln(tokens, strcount(tokens)));
}

LINEBLOCK* opvar(SCOPE* s, char* op, const char* name) {
	VAR* v = getvar(s, name);
	return opvarraw(s, op, v);
}

LINEBLOCK* pushvarraw(SCOPE*s, VAR* v) {
	return opvarraw(s, "push", v);
}

LINEBLOCK* pushvar(SCOPE* s, const char* name) {
	return opvar(s, "push", name);
}

LINEBLOCK* popvar(SCOPE* s, const char* name) {
	return opvar(s, "pop", name);
}

LINEBLOCK* pushfalse() {
	return pushconstant(0);
}

LINEBLOCK* pushtrue() {
	LINEBLOCK* blk = pushfalse();
	appendln(blk, onetoken("not"));
	return blk;
}

LINEBLOCK* pushthis() {
	char* pushthis[] = { "push", "pointer", "0" };
	return mklnblk(mksimpleln(pushthis, strcount(pushthis)));
}

LINEBLOCK* compilekeywordconst(SCOPE* s, TERM* t) {
	if(!strcmp(t->string, "true")) return pushtrue();
	if(!strcmp(t->string, "false")) return pushfalse();
	if(!strcmp(t->string, "this")) return pushthis();
	eprintf("Unsupported keyword '%s'\n", t->string);
	exit(1);
}

LINEBLOCK* compileterm(SCOPE* s, TERM* t) {
	if(t->type == intconstant) return pushconstant(t->integer);
	if(t->type == unaryopterm) return pushunaryopterm(s, t);
	if(t->type == innerexpression) return compileexpression(s, t->expression);
	if(t->type == varname) return pushvar(s, t->string);
	if(t->type == subroutcall) return compilesubroutcall(s, t->call);
	if(t->type == keywordconstant) return compilekeywordconst(s, t);
	else {
		eprintf("Unsupported term yet %i\n", t->type);
		exit(1);
	}
}

LINEBLOCK* compileexpression(SCOPE* s, TERM* e) {
	LINEBLOCK* blk = compileterm(s, e);
	TERM* curr = e->next;
	if(curr != NULL) {
		while(true) {
			blk = mergelnblks(blk, compileterm(s, curr));
			if(curr->next != NULL) {
				appendln(blk, mathopln(curr->op));
				curr = curr->next;
			}
			else break;
		}
		appendln(blk, mathopln(e->op));
	}
	return blk;
}

LINEBLOCK* compileexplist(SCOPE* s, EXPRESSIONLIST* explist) {
	LINEBLOCK* head = NULL;
	while(explist != NULL) {
		head = mergelnblks(head, compileexpression(s, explist->expression));
		explist = explist->next;
	}
	return head;
}

LINEBLOCK* compilecallln(SCOPE* s, SUBROUTDEC* d, SUBROUTCALL* call) {
	LINE* ln = mkline(3);

	addtoken(ln, ezheapstr("call"));

	addtoken(ln, dotlabel(d->class->name, call->name));

	int count = countexpressions(call->parameters);
	if(d->subroutclass == method)
		count++;
	addtoken(ln, itoa(count));

	return mklnblk(ln);
}

LINEBLOCK* compilesubroutcall(SCOPE* s, SUBROUTCALL* call) {
	VAR* v;
	SUBROUTDEC* d = getsubroutdecfromcall(s, call, &v);
	LINEBLOCK* blk = compilecallln(s, d, call);

	if(call->parameters != NULL)
		blk = mergelnblks(compileexplist(s, call->parameters), blk);

	if(d->subroutclass == method) {
		if(call->parentname == NULL)
			blk = mergelnblks(pushthis(), blk);
		else
			blk = mergelnblks(pushvarraw(s, v), blk);
	}

	// void functions always return 0
	// therefore must be thrown away
	if(!strcmp(d->type, "void")) {
		char* tokens[] = { "pop", "temp", "0" };
		appendln(blk, mksimpleln(tokens, sizeof(tokens) / sizeof(char*)));
	}

	return blk;
}

LINEBLOCK* compileret(SCOPE* s, TERM* e) {
	LINE* ret = onetoken("return");
	LINEBLOCK* blk = mklnblk(ret);

	// void subroutdecs return 0
	if(e == NULL) {
		char* tokens[] = { "push", "constant", "0" };
		appendlnbefore(blk, mksimpleln(tokens, strcount(tokens)));
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
	appendln(blk, mksimpleln(ifgoto, strcount(ifgoto)));
	
	char* falselabel = mkcondlabel("IF_FALSE", mycount);
	char* gotofalse[] = { "goto", falselabel };
	appendln(blk, mksimpleln(gotofalse, strcount(gotofalse)));

	char* truelabelln[] = { "label", truelabel };
	appendln(blk, mksimpleln(truelabelln, strcount(truelabelln)));

	blk = mergelnblks(blk, compilestatements(c, s, st->base->statements));

	char* endlabel;
	bool haselse = st->elsestatements != NULL;
	if(haselse) {
		endlabel = mkcondlabel("IF_END", mycount);
		char* endgoto[] = { "goto", endlabel };
		appendln(blk, mksimpleln(endgoto, strcount(endgoto)));
	}

	char* falselabelln[] = { "label", falselabel};
	appendln(blk, mksimpleln(falselabelln, strcount(falselabelln)));

	if(haselse) {
		blk = mergelnblks(blk, compilestatements(c, s, st->elsestatements));
		char* endlabelln[] = { "label", endlabel };
		appendln(blk, mksimpleln(endlabelln, strcount(endlabelln)));
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
	appendlnbefore(blk, mksimpleln(explabelln, strcount(explabelln)));

	appendln(blk, onetoken("not"));

	char* endlabel = mkcondlabel("WHILE_END", mycount);
	char* ifgoto[] = { "if-goto", endlabel };
	appendln(blk, mksimpleln(ifgoto, strcount(ifgoto)));

	blk = mergelnblks(blk, compilestatements(c, s, w->statements));

	char* gotoln[] = { "goto", explabel };
	appendln(blk, mksimpleln(gotoln, strcount(gotoln)));

	char* endlabelln[] = { "label", endlabel };
	appendln(blk, mksimpleln(endlabelln, strcount(endlabelln)));

	return blk;
}

LINEBLOCK* compilelet(SCOPE* s, LETSTATEMENT* l) {
	// missing array ind
	LINEBLOCK* blk = compileexpression(s, l->expression);
	blk = mergelnblks(blk, popvar(s, l->varname));
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
	LINE* label = mksimpleln(labelstrs, strcount(labelstrs));
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
	appendln(blk, mksimpleln(size, strcount(size)));
	appendln(blk, mksimpleln(memalloc, strcount(memalloc)));
	appendln(blk, mksimpleln(poppointer, strcount(poppointer)));
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
	appendln(blk, mksimpleln(pusharg0, strcount(pusharg0)));
	appendln(blk, mksimpleln(poppointer, strcount(poppointer)));

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
		addparameters(myscope, sd->parameters);
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
