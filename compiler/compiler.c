#include <stdlib.h>
#include <string.h>
#include "compiler.h"

LINEBLOCK* compilestatements(SCOPE* s, CLASS* c, STATEMENT* sts);

int countparameters(EXPRESSIONLIST* params) {
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
		i++;
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

char* mkcondlabel(SCOPE* s) {
	char* label = dotlabel("cond-label", itoa(s->condlabelcount));
	s->condlabelcount++;
	return label;
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

LINEBLOCK* compileexpression(SCOPE* s, TERM* e) {
	LINEBLOCK* myblk;
	LINEBLOCK* next = NULL;

	if(e->type == intconstant) {
		char* tokens[] = { "push", "constant", itoa(e->integer) };
		myblk = mklnblk(mksimpleln(tokens, strcount(tokens)));
	}
	else if(e->type == unaryopterm) {
		myblk = compileexpression(s, e->expression);
		LINE* neg = onetoken("neg");
		appendln(myblk, neg);
	}
	else if(e->type == innerexpression) {
		myblk = compileexpression(s, e->expression);
	}
	else if(e->type == varname) {
		eprintf("TO BE IMPLEMENTED\n");
		exit(1);
	}
	else {
		eprintf("Unsupported term yet %i\n", e->type);
		exit(1);
	}

	if(e->next != NULL) {
		next = compileexpression(s, e->next);
		appendln(next, mathopln(e->op));
		myblk = mergelnblks(myblk, next);
	}

	return myblk;
}

LINEBLOCK* compileparameters(SCOPE* s, EXPRESSIONLIST* params) {
	LINEBLOCK* head = NULL;
	while(params != NULL) {
		head = mergelnblks(head, compileexpression(s, params->expression));
		params = params->next;
	}
	return head;
}

LINEBLOCK* compilecallln(CLASS* c, SUBROUTCALL* call) {
	LINE* ln = mkline(3);

	addtoken(ln, ezheapstr("call"));

	if(call->parentname != NULL)
		addtoken(ln, dotlabel(call->parentname, call->name));
	else
		addtoken(ln, dotlabel(c->name, call->name));

	addtoken(ln, itoa(countparameters(call->parameters)));

	return mklnblk(ln);
}

// temporary ignore list for OS functions
char* ignoresubroutdecs[] = {
	"printInt", "void", "peek", "int"
};
int ignorecount = sizeof(ignoresubroutdecs) / sizeof(char*);

LINEBLOCK* compilesubroutcall(SCOPE* s, CLASS* c, SUBROUTCALL* call) {
	LINEBLOCK* block = compilecallln(c, call);

	if(call->parameters != NULL)
		block = mergelnblks(compileparameters(s, call->parameters), block);

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
		appendln(block, mksimpleln(tokens, sizeof(tokens) / sizeof(char*)));
	}

	return block;
}

LINEBLOCK* compileret(SCOPE* s, TERM* e) {
	LINE* ret = onetoken("return");
	LINEBLOCK* block = mklnblk(ret);

	// void subroutdecs return 0
	if(e == NULL) {
		char* tokens[] = { "push", "constant", "0" };
		appendlnbefore(block, mksimpleln(tokens, strcount(tokens)));
	} else
		block = mergelnblks(compileexpression(s, e), block);

	return block;
}

LINEBLOCK* compileif(SCOPE* s, CLASS* c, IFSTATEMENT* st) {
	LINEBLOCK* block = compileexpression(s, st->base->expression);
	appendln(block, onetoken("not"));
	
	char* label1 = mkcondlabel(s);
	char* ifgoto[] = { "if-goto", label1 };
	appendln(block, mksimpleln(ifgoto, strcount(ifgoto)));

	block = mergelnblks(block, compilestatements(s, c, st->base->statements));

	char* label2;
	bool haselse = st->elsestatements != NULL;
	if(haselse) {
		char* label2 = mkcondlabel(s);
		char* gotoln[] = { "goto", label2 };
		appendln(block, mksimpleln(gotoln, strcount(gotoln)));
	}

	char* label1ln[] = { "label", label1 };
	appendln(block, mksimpleln(label1ln, strcount(label1ln)));

	if(haselse) {
		block = mergelnblks(block, compilestatements(s, c, st->elsestatements));
		char* label2ln[] = { "label", label2 };
		appendln(block, mksimpleln(label2ln, strcount(label2ln)));
	}

	return block;
}

LINEBLOCK* compilewhile(SCOPE* s, CLASS* c, CONDSTATEMENT* w) {
	LINEBLOCK* block = compileexpression(s, w->expression);

	char* label1 = mkcondlabel(s);
	char* label1ln[] = { "label", label1 };
	appendlnbefore(block, mksimpleln(label1ln, strcount(label1ln)));

	appendln(block, onetoken("not"));

	char* label2 = mkcondlabel(s);
	char* ifgoto[] = { "if-goto", label2 };
	appendln(block, mksimpleln(ifgoto, strcount(ifgoto)));

	block = mergelnblks(block, compilestatements(s, c, w->statements));

	char* gotoln[] = { "goto", label1 };
	appendln(block, mksimpleln(gotoln, strcount(gotoln)));

	char* label2ln[] = { "label", label2 };
	appendln(block, mksimpleln(label2ln, strcount(label2ln)));

	return block;
}

LINEBLOCK* compilelet(SCOPE* s, CLASS* c, LETSTATEMENT* l) {
}

LINEBLOCK* compilestatement(SCOPE* s, CLASS* c, STATEMENT* st) {
	if(st->type == dostatement) return compilesubroutcall(s, c, st->dostatement);
	if(st->type == returnstatement) return compileret(s, st->retstatement);
	if(st->type == ifstatement) return compileif(s, c, st->ifstatement);
	if(st->type == whilestatement) return compilewhile(s, c, st->whilestatement);
	if(st->type == letstatement) return compilelet(s, c, st->letstatement);
	eprintf("UNSUPPORTED type %i\n", st->type);
	exit(1);
}

LINEBLOCK* compilestatements(SCOPE* s, CLASS* c, STATEMENT* sts) {
	LINEBLOCK* head = NULL;
	while(sts != NULL) {
		head = mergelnblks(head, compilestatement(s, c, sts));
		sts = sts->next;
	}
	return head;
}

LINEBLOCK* compilefunbody(SCOPE* s, CLASS* c, SUBROUTBODY* b) {
	SCOPE* myscope = mkscope(s);
	if(b->vardecs != NULL)
		addvardecs(s, b->vardecs);
	LINEBLOCK* head = compilestatements(myscope, c, b->statements);
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
	if(sd->subroutclass == function)
		return compilefundec(s, c, sd);
}

LINEBLOCK* compileclass(COMPILER* c, CLASS* class) {
	SCOPE* topscope = mkscope(c->globalscope);
	if(class->vardecs != NULL)
		addclassvardecs(topscope, class->vardecs);
	if(class->subroutdecs != NULL)
		addsubroutdecs(topscope, class->subroutdecs);

	LINEBLOCK* output = NULL;
	SUBROUTDEC* curr = class->subroutdecs;
	while(curr != NULL) {
		output = mergelnblks(output, compilesubroutdec(topscope, class, curr));
		curr = curr->next;
	}
	return output;
}

void compile(COMPILER* c) {
	LINEBLOCK* output = NULL;
	CLASS* curr = c->classes;
	while(curr != NULL) {
		output = mergelnblks(output, compileclass(c, curr));
		curr = curr->next;
	}
	c->output = output;
}

COMPILER* mkcompiler(CLASS* classes) {
	COMPILER* c = (COMPILER*)malloc(sizeof(COMPILER));
	c->globalscope = mkscope(NULL);
	addclasses(c->globalscope, classes);
	c->classes = classes;
	return c;
}
