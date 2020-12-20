#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "compiler.h"

typedef enum {
	subdec, classvardec, vardec, cl
} OBJTYPE;

void addtoken(LINE* l, char* token) {
	l->tokens[l->tokenscount] = token;
	l->tokenscount++;
}

bool existclass(CLASS* c, char* name) {
	CLASS* current = c;
	while(current != NULL) {
		if(!strcmp(current->name, name))
				return true;
		current = current->next;
	}
	return false;
}

void doubledeclarationmsg(char* name, char* f1, int l1, char* f2, int l2) {
	fprintf(stderr, "Double declaration of '%s' at '%s', line %i; previously defined at '%s', line %i\n", name, f1, l1, f2, l2);
	exit(1);
}

void xtractinfo(void* obj, OBJTYPE type, char** file, int* line) {
	if(type == classvardec) {
		*file = ((CLASSVARDEC*)obj)->base->file;
		*line = ((CLASSVARDEC*)obj)->base->definedat;
	}
	else if(type == vardec) {
		*file = ((VARDEC*)obj)->file;
		*line = ((VARDEC*)obj)->definedat;
	}
	else if(type == subdec) {
		*file = ((SUBDEC*)obj)->file;
		*line = ((SUBDEC*)obj)->definedat;
	}
	else if(type == cl) {
		*file = ((CLASS*)obj)->file;
		*line = ((CLASS*)obj)->definedat;
	}
}

void doubledeclaration(char* name, void* o1, OBJTYPE t1, void* o2, OBJTYPE t2) {
	char* f1;
	char* f2;
	int l1, l2;
	xtractinfo(o1, t1, &f1, &l1);
	xtractinfo(o2, t2, &f2, &l2);
	doubledeclarationmsg(name, f1, l1, f2, l2);
}

SCOPE* mkscope(SCOPE* prev) {
	SCOPE* s = (SCOPE*)malloc(sizeof(SCOPE));
	s->subroutines = NULL;
	s->classvardecs = NULL;
	s->vardecs = NULL;
	s->classes = NULL;
	s->previous = prev;
	return s;
}

void popscope(SCOPE** s) {
	SCOPE* prev = (*s)->previous;
	free(*s);
	(*s) = prev;
}

bool existstr(STRINGLIST* strs, char* str) {
	STRINGLIST* current = strs;
	while(current != NULL) {
		if(!strcmp(current->content, str))
			return true;
		current = current->next;
	}
	return false;
}

VARDEC* getvardec(SCOPE* s, char* name) {
	VARDEC* current = s->vardecs;
	while(current != NULL) {
		if(existstr(current->names, name))
			return current;
		current = current->next;
	}
	if(s->previous != NULL)
		return getvardec(s->previous, name);
	return NULL;
}

CLASSVARDEC* getclassvardec(SCOPE* s, char* name) {
	CLASSVARDEC* current = s->classvardecs;
	while(current != NULL) {
		if(existstr(current->base->names, name))
		       return current;
		current = current->next;
	}
	if(s->previous != NULL)
		return getclassvardec(s->previous, name);
	return NULL;
}

SUBDEC* getsubdec(SCOPE* s, char* name) {
	SUBDEC* current = s->subroutines;
	while(current != NULL) {
		if(!strcmp(current->name, name))
			return current;
		current = current->next;
	}
	if(s->previous != NULL)
		return getsubdec(s->previous, name);
	return NULL;
}

SUBDEC* getsubdecfromclass(CLASS* c, char* name) {
	SUBDEC* current = c->subdecs;
	while(current != NULL) {
		if(!strcmp(current->name, name))
			return current;
		current = current->next;
	}
	return NULL;
}

CLASS* getclass(SCOPE* s, char* name) {
	CLASS* current = s->classes;
	while(current != NULL) {
		if(!strcmp(current->name, name))
				return current;
		current = current->next;
	}
	if(s->previous != NULL)
		return getclass(s->previous, name);
	return NULL;
}

void* getbyname(SCOPE* s, OBJTYPE* t, char* name) {
	SUBDEC* sd = getsubdec(s, name);
	if(sd != NULL) {
		*t = subdec;
		return sd;
	}
	CLASSVARDEC* cvd = getclassvardec(s, name);
	if(cvd != NULL) {
		*t = classvardec;
		return cvd;
	}
	VARDEC* vd = getvardec(s, name);
	if(vd != NULL) {
		*t = vardec;
		return vd;
	}
	CLASS* c = getclass(s, name);
	if(c != NULL) {
		*t = cl;
		return c;
	}
	return NULL;
}

void* getbynamelist(SCOPE* s, STRINGLIST* names, OBJTYPE* t, char** name) {
	STRINGLIST* current = names;
	while(current != NULL) {
		void* obj = getbyname(s, t, current->content);
		if(obj != NULL) {
			*name = current->content;
			return obj;
		}
		current = current->next;
	}
	if(s->previous != NULL)
		return getbynamelist(s->previous, names, t, name);
	return NULL;
}

void addclassvardec(SCOPE* s, CLASSVARDEC* v) {
	OBJTYPE type;
	char* name;
	void* tmp = getbynamelist(s, v->base->names, &type, &name);
	if(tmp != NULL)
		doubledeclaration(name, v, classvardec, tmp, type);
	v->next = s->classvardecs;
	s->classvardecs = v;
}

void addvardec(SCOPE* s, VARDEC* v) {
	OBJTYPE type;
	char* name;
	void* tmp = getbynamelist(s, v->names, &type, &name);
	if(tmp != NULL)
		doubledeclaration(name, v, vardec, tmp, type);
	v->next = s->vardecs;
	s->vardecs = v;
}

void addsubdec(SCOPE* s, SUBDEC* sd) {
	OBJTYPE type;
	void* tmp = getbyname(s, &type, sd->name);
	if(tmp != NULL)
		doubledeclaration(sd->name, sd, subdec, tmp, type);
	sd->next = s->subroutines;
	s->subroutines = sd;
}

void addclass(SCOPE* s, CLASS* c) {
	OBJTYPE type;
	void* tmp = getbyname(s, &type, c->name);
	if(tmp != NULL)
		doubledeclaration(c->name, c, cl, tmp, type);
	c->next = s->classes;
	s->classes = c;
}

void addclassvardecs(SCOPE* s, CLASSVARDEC* vs) {
	CLASSVARDEC* current = vs;
	while(current != NULL) {
		addclassvardec(s, current);
		current = current->next;
	}
}

void addvardecs(SCOPE* s, VARDEC* vs) {
	VARDEC* current = vs;
	while(current != NULL) {
		addvardec(s, current);
		current = current->next;
	}
}

void addsubdecs(SCOPE* s, SUBDEC* ss) {
	SUBDEC* current = ss;
	while(current != NULL) {
		addsubdec(s, current);
		current = current->next;
	}
}

void addclasses(SCOPE* s, CLASS* c) {
	CLASS* current = c;
	while(current != NULL) {
		addclass(s, current);
		current = current->next;
	}
}

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
	snprintf(result, sz, "%s.%s", n1, n2);
	return result;
}

char* subdecname(CLASS* c, SUBDEC* sd) {
	return dotlabel(c->name, sd->name);
}

SUBDEC* getsubdecfromparent(SCOPE* s, SUBROUTCALL* call) {
	SUBDEC* sd;
	OBJTYPE type;
	void* parent = getbyname(s, &type, call->parentname);
	if(type == cl)
		sd = getsubdecfromclass((CLASS*)parent, call->name);
	else {
		VARDEC* vd;
		if (type == classvardec)
			vd = ((CLASSVARDEC*)parent)->base;
		else if (type == vardec)
			vd = (VARDEC*)parent;
		else {
			fprintf(stderr, "Unexpected subroutine identifier; file '%s', line %i\n", call->file, call->definedat);
			exit(1);
		}
		if(vd->primitive) {
			fprintf(stderr, "Primitive type doesn't have methods; file '%s', line %i\n", call->file, call->definedat);
			exit(1);
		}
		sd = getsubdecfromparent(s, call);
	}
	return sd;
}

LINE* onetoken(char* str) {
	LINE* ln = mkline(1);
	addtoken(ln, ezheapstr(str));
	return ln;
}

LINE* mksimpleln(char** tokens) {
	int count = sizeof(tokens) / sizeof(char*);

	LINE* ln = mkline(count);
	for(int i = 0; i < count; i++)
		addtoken(ln, ezheapstr(tokens[i]));

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
		return mksimpleln(tokens);
	}
	if(op == '*') {
		char* tokens[] = { "call", "Math.multiply", "2" };
		return mksimpleln(tokens);
	}
}

LINE* compileexpression(SCOPE* s, TERM* e, LINE** tail) {
	LINE* nexts = NULL;
	LINE* nextstail;
	LINE* r;
	
	if(e->next != NULL) {
		nexts = compileexpression(s, e->next, &nextstail);
		LINE* op = mathopln(e->op);
		nextstail->next = op;
		nextstail = op;
		op->next = NULL;
	}

	if(e->type == intconstant) {
		r = mkline(3);
		addtoken(r, ezheapstr("push"));
		addtoken(r, ezheapstr("constant"));
		addtoken(r, itoa(e->integer));
	}
	else if(e->type == unaryopterm) {
		r = mkline(1);
		addtoken(r, ezheapstr("neg"));
	}
	else if(e->type == innerexpression) {
		r = compileexpression(s, e->expression, tail); // might be wrong tail
	}
	else {
		fprintf(stderr, "Unsuported SHIT %i\n", e->type);
		exit(1);
	}

	if(nexts != NULL) {
		r->next = nexts;
		(*tail) = nextstail;
	}
	else {
		(*tail) = r;
		r->next = NULL;
	}
	return r;
}

LINE* compileparameters(SCOPE* s, EXPRESSIONLIST* ps, LINE** tail) {
	LINE* head;
	LINE* mytail;
	if(ps != NULL)
		head = compileexpression(s, ps->expression, &mytail);
	LINE* currln = head;
	EXPRESSIONLIST* current = ps->next;
	while(current != NULL) {
		LINE* newln = compileexpression(s, current->expression, &mytail);
		current = current->next;
		currln->next = newln;
		currln = newln;
	}
	(*tail) = mytail;
	return head;
}

LINE* compilesubroutcall(SCOPE* s, CLASS* c, SUBROUTCALL* call) {
	/* FOR NOW THERE IS NO OS SO THIS WILL CAUSE PROBLEMS
	SUBDEC* sd;
	if(call->parentname != NULL)
		sd = getsubdecfromparent(s, call);
	else
		sd = getsubdec(s, call->name);
	if(sd == NULL) {
		fprintf(stderr, "Method '%s' does not exist; file '%', line %i\n", call->name, call->file, call->definedat);
		exit(1);
	}
	*/

	// At the moment can only call functions
	LINE* tail;
	LINE* head = compileparameters(s, call->parameters, &tail);

	LINE* callvm = mkline(3);
	addtoken(callvm, ezheapstr("call"));
	if(call->parentname != NULL)
		addtoken(callvm, dotlabel(call->parentname, call->name));
	else
		addtoken(callvm, dotlabel(c->name, call->name));

	addtoken(callvm, itoa(countparameters(call->parameters)));

	tail->next = callvm;
	tail = callvm;

	return head;
}

LINE* compileret(SCOPE* s, TERM* e) {
	// missing expression handling
	if(e == NULL) {
		LINE* r = mkline(1);
		addtoken(r, ezheapstr("return"));
		return r;
	}
}

LINE* compilestatement(SCOPE* s, CLASS* c, STATEMENT* st) {
	if(st->type == dostatement)
		return compilesubroutcall(s, c, st->dost);
	else if(st->type == returnstatement)
		return compileret(s, st->retst);
	else {
		fprintf(stderr, "UNSUPPORTED\n");
		exit(1);
	}
}

LINE* compilestatements(SCOPE* s, CLASS* c, STATEMENT* sts) {
	LINE* head;
	LINE* curr;
	if(sts != NULL) {
		head = compilestatement(s, c, sts);
		curr = head;
		sts = sts->next;
		while(sts != NULL) {
			LINE* ln = compilestatement(s, c, sts);
			curr->next = ln;
			curr = ln;
			sts = sts->next;
		}
	}
	return head;
}

LINE* compilefunbody(SCOPE* s, CLASS* c, SUBROUTBODY* b) {
	// missing scope and vardecs handling
	LINE* head = compilestatements(s, c, b->statements);
	return head;
}

LINE* compilefundec(SCOPE* s, CLASS* c, SUBDEC* f) {
	LINE* head = mkline(3);
	addtoken(head, ezheapstr("function"));
	addtoken(head, subdecname(c, f));
	addtoken(head, itoa(countlocalvars(f->body->vardecs)));

	head->next = compilefunbody(s, c, f->body);
	return head;
}

LINE* compilesubdec(SCOPE* s, CLASS* c, SUBDEC* sd) {
	// 'this' and arguments are pushed by caller
	// Must have a 'return' at the end
	// Label names must have class name too (see mapping)
	
	// types: method, function, constructor
	// must switch all of these
	if(sd->subroutclass == function)
		return compilefundec(s, c, sd);
}

void compileclass(COMPILER* c, CLASS* class) {
	SCOPE* topscope = mkscope(c->globalscope);
	addclassvardecs(topscope, class->vardecs);
	addsubdecs(topscope, class->subdecs);

	SUBDEC* current = class->subdecs;
	while(current != NULL) {
		compilesubdec(topscope, class, current);
		current = current->next;
	}
}

void compile(COMPILER* c) {
	CLASS* current = c->globalscope->classes;
	while(current != NULL) {
		compileclass(c, current);
		current = current->next;
	}
}

COMPILER* mkcompiler(CLASS* classes) {
	COMPILER* c = (COMPILER*)malloc(sizeof(COMPILER));
	c->globalscope = mkscope(NULL);
	addclasses(c->globalscope, classes);
	return c;
}
