#include <stdlib.h>
#include <string.h>
#include "compiler.h"

LINEBLOCK* compilestatements(SCOPE* s, STATEMENT* sts);
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

LINEBLOCK* opvar(SCOPE* s, char* op, const char* name) {
	VAR* v = getvar(s, name);
	char* tokens[] = { op, v->memsegment, itoa(v->index) };
	return mklnblk(mksimpleln(tokens, strcount(tokens)));

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

LINEBLOCK* compilekeywordconst(SCOPE* s, TERM* t) {
	if(!strcmp(t->string, "true")) return pushtrue();
	if(!strcmp(t->string, "false")) return pushfalse();
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

LINEBLOCK* compilecallln(SCOPE* s, SUBROUTCALL* call) {
	LINE* ln = mkline(3);

	addtoken(ln, ezheapstr("call"));

	if(call->parentname != NULL)
		addtoken(ln, dotlabel(call->parentname, call->name));
	else
		addtoken(ln, dotlabel(s->currclass->name, call->name));

	addtoken(ln, itoa(countexpressions(call->parameters)));

	return mklnblk(ln);
}

// temporary ignore list for OS functions
char* ignoresubroutdecs[] = {
	"printInt", "void", "peek", "int", "poke", "void"
};
int ignorecount = sizeof(ignoresubroutdecs) / sizeof(char*);

LINEBLOCK* compilesubroutcall(SCOPE* s, SUBROUTCALL* call) {
	LINEBLOCK* blk = compilecallln(s, call);

	if(call->parameters != NULL)
		blk = mergelnblks(compileexplist(s, call->parameters), blk);

	// void functions always return 0
	// therefore must be thrown away

	// gambiarra
	char* type = NULL;
	for(int i = 0; i < ignorecount; i += 2) {
		if(!strcmp(call->name, ignoresubroutdecs[i])) {
			type = ignoresubroutdecs[i+1];
			break;
		}
	}
	if(type == NULL)
		type = getsubroutdecfromcall(s, call)->type;
	if(!strcmp(type, "void")) {
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

LINEBLOCK* compileif(SCOPE* s, IFSTATEMENT* st) {
	LINEBLOCK* blk = compileexpression(s, st->base->expression);

	static int ifcount = 0;
	int mycount = ifcount;
	ifcount++;
	
	char* truelabel = mkcondlabel("IF_TRUE", mycount);
	char* ifgoto[] = { "if-goto", truelabel };
	appendln(blk, mksimpleln(ifgoto, strcount(ifgoto)));
	
	char* falselabel = mkcondlabel("IF_FALSE", mycount);
	char* gotofalse[] = { "goto", falselabel };
	appendln(blk, mksimpleln(gotofalse, strcount(gotofalse)));

	char* truelabelln[] = { "label", truelabel };
	appendln(blk, mksimpleln(truelabelln, strcount(truelabelln)));

	blk = mergelnblks(blk, compilestatements(s, st->base->statements));

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
		blk = mergelnblks(blk, compilestatements(s, st->elsestatements));
		char* endlabelln[] = { "label", endlabel };
		appendln(blk, mksimpleln(endlabelln, strcount(endlabelln)));
	}


	return blk;
}

LINEBLOCK* compilewhile(SCOPE* s, CONDSTATEMENT* w) {
	LINEBLOCK* blk = compileexpression(s, w->expression);

	static int whilecount = 0;
	int mycount = whilecount;
	whilecount++;

	char* explabel = mkcondlabel("WHILE_EXP", mycount);
	char* explabelln[] = { "label", explabel };
	appendlnbefore(blk, mksimpleln(explabelln, strcount(explabelln)));

	appendln(blk, onetoken("not"));

	char* endlabel = mkcondlabel("WHILE_END", mycount);
	char* ifgoto[] = { "if-goto", endlabel };
	appendln(blk, mksimpleln(ifgoto, strcount(ifgoto)));

	blk = mergelnblks(blk, compilestatements(s, w->statements));

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

LINEBLOCK* compilestatement(SCOPE* s, STATEMENT* st) {
	s->currdebug = st->debug;
	if(st->type == dostatement) return compilesubroutcall(s, st->dostatement);
	if(st->type == returnstatement) return compileret(s, st->retstatement);
	if(st->type == ifstatement) return compileif(s, st->ifstatement);
	if(st->type == whilestatement) return compilewhile(s, st->whilestatement);
	if(st->type == letstatement) return compilelet(s, st->letstatement);
	eprintf("UNSUPPORTED type %i\n", st->type);
	exit(1);
}

LINEBLOCK* compilestatements(SCOPE* s, STATEMENT* sts) {
	LINEBLOCK* head = NULL;
	while(sts != NULL) {
		head = mergelnblks(head, compilestatement(s, sts));
		sts = sts->next;
	}
	return head;
}

LINEBLOCK* compilefunbody(SCOPE* s, CLASS* c, SUBROUTBODY* b) {
	SCOPE* myscope = mkscope(s);
	myscope->currclass = c;
	if(b->vardecs != NULL)
		addlocalvars(s, b->vardecs);
	LINEBLOCK* head = compilestatements(myscope, b->statements);
	return head;
}

LINEBLOCK* compilefundec(SCOPE* s, CLASS* c, SUBROUTDEC* f) {
	LINE* label = mkline(3);
	addtoken(label, ezheapstr("function"));
	addtoken(label, dotlabel(c->name, f->name));
	addtoken(label, itoa(countlocalvars(f->body->vardecs)));
	label->next = NULL;

	if(f->body->statements != NULL) {
		LINEBLOCK* body = compilefunbody(s, c, f->body);
		appendlnbefore(body, label);
		return body;
	}
	else
		return mklnblk(label);
}

LINEBLOCK* compilesubroutdec(SCOPE* s, CLASS* c, SUBROUTDEC* sd) {
	// 'this' and arguments are pushed by caller
	// Must have a 'return' at the end
	// Label names must have class name too (see mapping)
	
	// types: method, function, constructor
	// must switch all of these
	SCOPE* myscope = mkscope(s);
	if(sd->parameters != NULL)
		addparameters(myscope, sd->parameters);
	if(sd->subroutclass == function)
		return compilefundec(myscope, c, sd);
}

LINEBLOCK* compileclass(COMPILER* c, CLASS* class) {
	SCOPE* topscope = mkscope(c->globalscope);
	if(class->vardecs != NULL)
		addclassvardecs(topscope, class->vardecs);
	if(class->subroutdecs != NULL)
		topscope->subroutines = class->subroutdecs;

	LINEBLOCK* output = NULL;
	SUBROUTDEC* curr = class->subroutdecs;
	while(curr != NULL) {
		output = mergelnblks(output, compilesubroutdec(topscope, class, curr));
		curr = curr->next;
	}
	return output;
}

COMPILER* mkcompiler(CLASS* classes) {
	COMPILER* c = (COMPILER*)malloc(sizeof(COMPILER));
	c->globalscope = mkscope(NULL);
	c->globalscope->classes = classes;
	c->classes = classes;
	return c;
}
