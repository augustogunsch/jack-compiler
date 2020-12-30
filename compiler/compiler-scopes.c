#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "compiler-scopes.h"

typedef enum { local, staticseg, arg, fieldseg } MEMSEGMENT;
char* memsegnames[] = { "local", "static", "argument", "this" };

// Error messages
void doubledeclaration(const char* name, DEBUGINFO* d1, DEBUGINFO* d2);
void ensurenoduplicate(SCOPE* s, char* name);

// Getters
VAR* getvarinvars(VAR* vars, const char* name);
CLASS* getclass(SCOPE* s, const char* name);
SUBROUTDEC* getsubroutdecfromlist(SUBROUTDEC* start, char* name);
SUBROUTDEC* getmethod(SCOPE* s, VAR* parent, SUBROUTCALL* call);
SUBROUTDEC* getfunction(SCOPE* s, SUBROUTCALL* call);
SUBROUTDEC* getsubroutdecwithparent(SCOPE* s, SUBROUTCALL* call);
SUBROUTDEC* getsubroutdecwithoutparent(SCOPE* s, SUBROUTCALL* call);
SUBROUTDEC* getsubroutdec(SCOPE* s, const char* name);

// Scope adding
VAR* mkvar(char* type, char* name, bool primitive, DEBUGINFO* debug, MEMSEGMENT seg);
void addvar(SCOPE* s, VAR** dest, VAR* v);
void addlocalvar(SCOPE* s, VARDEC* v);
void addclassvardec(SCOPE* s, CLASSVARDEC* v);
void addparameter(SCOPE* s, PARAMETER* p);

// Error messages
void doubledeclaration(const char* name, DEBUGINFO* d1, DEBUGINFO* d2) {
	eprintf("Double declaration of '%s' at '%s', line %i; previously defined at '%s', line %i\n",
				name, d1->file, d1->definedat, d2->file, d2->definedat);
	exit(1);
}

void notdeclared(const char* name, DEBUGINFO* debug) {
	eprintf("'%s' not declared; file '%s', line %i\n", name, debug->file, debug->definedat);
	exit(1);
}

void invalidparent(SUBROUTCALL* call) {
	eprintf("Invalid subroutine parent '%s'; file '%s', line %i\n", call->parentname, call->debug->file, call->debug->definedat);
	exit(1);
}

void ensurenoduplicate(SCOPE* s, char* name) {
	VAR* v = getvar(s, name);
	if(v != NULL)
		doubledeclaration(name, s->currdebug, v->debug);

	CLASS* c = getclass(s, name);
	if(c != NULL)
		doubledeclaration(name, s->currdebug, c->debug);

	SUBROUTDEC* sr = getsubroutdec(s, name);
	if(sr != NULL)
		doubledeclaration(name, s->currdebug, sr->debug);
}

// Scope handling
SCOPE* mkscope(SCOPE* prev) {
	SCOPE* s = (SCOPE*)malloc(sizeof(SCOPE));
	s->previous = prev;
	s->localvars = NULL;
	s->fields = NULL;
	s->staticvars = NULL;
	s->parameters = NULL;
	s->classes = NULL;
	s->subroutines = NULL;
	return s;
}

// Getters
VAR* getvarinvars(VAR* vars, const char* name) {
	while(vars != NULL) {
		if(!strcmp(vars->name, name))
			return vars;
		vars = vars->next;
	}
	return NULL;
}

VAR* getvar(SCOPE* s, const char* name) {
	VAR* var = getvarinvars(s->localvars, name);
	if(var != NULL)
		return var;
	var = getvarinvars(s->parameters, name);
	if(var != NULL)
		return var;
	var = getvarinvars(s->fields, name);
	if(var != NULL)
		return var;
	var = getvarinvars(s->staticvars, name);
	if(var != NULL)
		return var;
	if(s->previous != NULL)
		return getvar(s->previous, name);
	return NULL;
}

CLASS* getclass(SCOPE* s, const char* name) {
	CLASS* curr = s->classes;
	while(curr != NULL) {
		if(!strcmp(curr->name, name))
			return curr;
		curr = curr->next;
	}
	if(s->previous != NULL)
		return getclass(s->previous, name);
	return NULL;
}

SUBROUTDEC* getsubroutdecfromlist(SUBROUTDEC* start, char* name) {
	while(start != NULL) {
		if(!strcmp(start->name, name))
			return start;
		start = start->next;
	}
	return NULL;
}

SUBROUTDEC* getmethod(SCOPE* s, VAR* parent, SUBROUTCALL* call) {
	CLASS* c = getclass(s, parent->type);
	SUBROUTDEC* d = getsubroutdecfromlist(c->subroutdecs, call->name);
	if(d == NULL)
		notdeclared(call->name, call->debug);
	if(d->subroutclass != method) {
		eprintf("Calling a function/constructor as if it were a method; file '%s', line %i\n", call->debug->file, call->debug->definedat);
		exit(1);
	}
	return d;
}

SUBROUTDEC* getfunction(SCOPE* s, SUBROUTCALL* call) {
	CLASS* c = getclass(s, call->parentname);
	if(c == NULL)
		notdeclared(call->parentname, call->debug);
	SUBROUTDEC* d = getsubroutdecfromlist(c->subroutdecs, call->name);
	if(d == NULL)
		notdeclared(call->name, call->debug);
	if(d->subroutclass == method) {
		eprintf("Calling a method as if it were a function; file '%s', line %i\n", call->debug->file, call->debug->definedat);
		exit(1);
	}
	return d;
}

SUBROUTDEC* getsubroutdecwithparent(SCOPE* s, SUBROUTCALL* call) {
	VAR* parent = getvar(s, call->parentname);
	if(parent != NULL)
		return getmethod(s, parent, call);
	else 
		return getfunction(s, call);
}

SUBROUTDEC* getsubroutdecwithoutparent(SCOPE* s, SUBROUTCALL* call) {
	SUBROUTDEC* d = getsubroutdecfromlist(s->currclass->subroutdecs, call->name);
	if(d == NULL)
		notdeclared(call->name, call->debug);
	return d;
}

SUBROUTDEC* getsubroutdecfromcall(SCOPE* s, SUBROUTCALL* call) {
	if(call->parentname != NULL)
		return getsubroutdecwithparent(s, call);
	else
		return getsubroutdecwithoutparent(s, call);
}

SUBROUTDEC* getsubroutdec(SCOPE* s, const char* name) {
	SUBROUTDEC* curr = s->subroutines;
	while(curr != NULL) {
		if(!strcmp(curr->name, name))
			return curr;
		curr = curr->next;
	}
	if(s->previous != NULL)
		return getsubroutdec(s->previous, name);
	return NULL;
}

// Scope adding
VAR* mkvar(char* type, char* name, bool primitive, DEBUGINFO* debug, MEMSEGMENT seg) {
	VAR* v = (VAR*)malloc(sizeof(VAR));
	v->name = name;
	v->type = type;
	v->debug = debug;
	v->memsegment = memsegnames[seg];
	v->primitive = primitive;
	return v;
}

void addvar(SCOPE* s, VAR** dest, VAR* v) {
	ensurenoduplicate(s, v->name);

	if(!v->primitive) {
		CLASS* type = getclass(s, v->type);
		if(type == NULL)
			notdeclared(v->type, v->debug);
	}

	if(*dest == NULL)
		v->index = 0;
	else
		v->index = 1+(*dest)->index;

	v->next = *dest;
	*dest = v;
}

void addlocalvar(SCOPE* s, VARDEC* v) {
	STRINGLIST* currname = v->names;
	while(currname != NULL) {
		addvar(s, &(s->localvars), mkvar(v->type, currname->content, v->primitive, v->debug, local));
		currname = currname->next;
	}
}

void addstaticvar(SCOPE* s, CLASSVARDEC* v) {
	STRINGLIST* currname = v->base->names;
	while(currname != NULL) {
		addvar(s, &(s->staticvars), mkvar(v->base->type, currname->content, v->base->primitive, v->base->debug, staticseg));
		currname = currname->next;
	}
}

void addfield(SCOPE* s, CLASSVARDEC* v) {
	STRINGLIST* currname = v->base->names;
	while(currname != NULL) {
		addvar(s, &(s->fields), mkvar(v->base->type, currname->content, v->base->primitive, v->base->debug, fieldseg));
		currname = currname->next;
	}
}

void addclassvardec(SCOPE* s, CLASSVARDEC* v) {
	if(v->type == staticseg)
		addstaticvar(s, v);
	else
		addfield(s, v);
}

void addparameter(SCOPE* s, PARAMETER* p) {
	addvar(s, &(s->parameters), mkvar(p->type, p->name, p->primitive, p->debug, arg));
}

// Group adding
void addclassvardecs(SCOPE* s, CLASSVARDEC* classvardecs) {
	while(classvardecs != NULL) {
		addclassvardec(s, classvardecs);
		classvardecs = classvardecs->next;
	}
}

void addlocalvars(SCOPE* s, VARDEC* localvars) {
	while(localvars != NULL) {
		addlocalvar(s, localvars);
		localvars = localvars->next;
	}
}

void addparameters(SCOPE* s, PARAMETER* params) {
	while(params != NULL) {
		addparameter(s, params);
		params = params->next;
	}
}
