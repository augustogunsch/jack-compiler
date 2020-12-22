#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "compiler-scopes.h"

// INTERNAL FUNCTIONS
// Information gathering
bool existstr(STRINGLIST* strs, const char* str);
bool existclass(CLASS* c, const char* name);
DEBUGINFO* getdebuginfo(OBJ* obj);

// Error messages
void doubledeclaration(const char* name, DEBUGINFO* debug, OBJ* other);
void ensurenoduplicate(SCOPE* s, const char* name, DEBUGINFO* debug);

// Scope handling
void popscope(SCOPE** s); // may be removed

// Single type getters
VARDEC* getvardec(SCOPE* s, const char* name);
CLASSVARDEC* getclassvardec(SCOPE* s, const char* name);

// Generic getters
OBJ* getbynamelist(SCOPE* s, STRINGLIST* names, const char** retname);

// Scope adding
void addclassvardec(SCOPE* s, CLASSVARDEC* v);
void addvardec(SCOPE* s, VARDEC* v);
void addsubroutdec(SCOPE* s, SUBROUTDEC* sd);
void addclass(SCOPE* s, CLASS* c);

// DEFINITIONS
// Information gathering
bool existstr(STRINGLIST* strs, const char* str) {
	while(strs != NULL) {
		if(!strcmp(strs->content, str))
			return true;
		strs = strs->next;
	}
	return false;
}

bool existclass(CLASS* c, const char* name) {
	while(c != NULL) {
		if(!strcmp(c->name, name))
				return true;
		c = c->next;
	}
	return false;
}

// OBJ handling

DEBUGINFO* getdebugclassvardec(OBJ* obj) {
	return obj->classvardec->base->debug;
}

DEBUGINFO* getdebugvardec(OBJ* obj) {
	return obj->vardec->debug;
}

DEBUGINFO* getdebugsubroutdec(OBJ* obj) {
	return obj->subroutdec->debug;
}

DEBUGINFO* getdebugclass(OBJ* obj) {
	return obj->class->debug;
}

VARDEC* tovardec(OBJ* obj) {
	if (obj->type == classvardec)
		return obj->classvardec->base;
	else if (obj->type == vardec)
		return obj->vardec;
	return NULL;
}

// Error messages

void doubledeclaration(const char* name, DEBUGINFO* debug, OBJ* other) {
	DEBUGINFO* debugother = other->getdebug(other);
	eprintf("Double declaration of '%s' at '%s', line %i; previously defined at '%s', line %i\n",
				name, debug->file, debug->definedat, debugother->file, debugother->definedat);
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

void ensurenoduplicate(SCOPE* s, const char* name, DEBUGINFO* debug) {
	OBJ* other = getbyname(s, name);
	if(other != NULL)
		doubledeclaration(name, debug, other);
}

void ensurenoduplicates(SCOPE* s, STRINGLIST* names, DEBUGINFO* debug) {
	const char* othername;
	OBJ* other = getbynamelist(s, names, &othername);
	if(other != NULL)
		doubledeclaration(othername, debug, other);
}

// Scope handling

SCOPE* mkscope(SCOPE* prev) {
	SCOPE* s = (SCOPE*)malloc(sizeof(SCOPE));
	s->subroutines = NULL;
	s->classvardecs = NULL;
	s->vardecs = NULL;
	s->classes = NULL;
	s->previous = prev;
	return s;
}

void popscope(SCOPE** s) { // might be useless
	SCOPE* prev = (*s)->previous;
	free(*s);
	(*s) = prev;
}

// Single type getters
VARDEC* getvardec(SCOPE* s, const char* name) {
	VARDEC* curr = s->vardecs;
	while(curr != NULL) {
		if(existstr(curr->names, name))
			return curr;
		curr = curr->next;
	}
	if(s->previous != NULL)
		return getvardec(s->previous, name);
	return NULL;
}

CLASSVARDEC* getclassvardec(SCOPE* s, const char* name) {
	CLASSVARDEC* curr = s->classvardecs;
	while(curr != NULL) {
		if(existstr(curr->base->names, name))
		       return curr;
		curr = curr->next;
	}
	if(s->previous != NULL)
		return getclassvardec(s->previous, name);
	return NULL;
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

SUBROUTDEC* getsubroutdecfromclass(CLASS* c, const char* name) {
	SUBROUTDEC* curr = c->subroutdecs;
	while(curr != NULL) {
		if(!strcmp(curr->name, name))
			return curr;
		curr = curr->next;
	}
	return NULL;
}

SUBROUTDEC* getsubroutdecfromvar(SCOPE* s, OBJ* var, SUBROUTCALL* call) {
	VARDEC* vd = tovardec(var);
	if(vd == NULL || vd->primitive)
		invalidparent(call);
	CLASS* c = getclass(s, vd->type);
	return getsubroutdecfromclass(c, call->name);
}

SUBROUTDEC* getsubroutdecfromparent(SCOPE* s, SUBROUTCALL* call) {
	SUBROUTDEC* sd;

	OBJ* parent = getbyname(s, call->parentname);
	if(parent == NULL)
		notdeclared(call->parentname, call->debug);

	if(parent->type == class)
		sd = getsubroutdecfromclass(parent->class, call->name);
	else
		sd = getsubroutdecfromvar(s, parent, call);
	return sd;
}

SUBROUTDEC* getsubroutdecfromcall(SCOPE* s, SUBROUTCALL* call) {
	SUBROUTDEC* sd;
	if(call->parentname != NULL)
		sd = getsubroutdecfromparent(s, call);
	else
		sd = getsubroutdec(s, call->name);
	if(sd == NULL)
		notdeclared(call->name, call->debug);
	return sd;
}

// Generic getters
OBJ* getbyname(SCOPE* s, const char* name) {
	OBJ* o = (OBJ*)malloc(sizeof(OBJ));

	CLASSVARDEC* cvd = getclassvardec(s, name);
	if(cvd != NULL) {
		o->classvardec = cvd;
		o->type = classvardec;
		o->getdebug = getdebugvardec;
		return o;
	}

	VARDEC* vd = getvardec(s, name);
	if(vd != NULL) {
		o->vardec = vd;
		o->type = vardec;
		o->getdebug = getdebugsubroutdec;
		return o;
	}

	SUBROUTDEC* sd = getsubroutdec(s, name);
	if(sd != NULL) {
		o->subroutdec = sd;
		o->type = subroutdec;
		o->getdebug = getdebugclassvardec;
		return o;
	}

	CLASS* c = getclass(s, name);
	if(c != NULL) {
		o->class = c;
		o->type = class;
		o->getdebug = getdebugclass;
		return o;
	}

	free(o);
	return NULL;
}

OBJ* getbynamelist(SCOPE* s, STRINGLIST* names, const char** retname) {
	while(names != NULL) {
		OBJ* o = getbyname(s, names->content);
		if(o != NULL) {
			*retname = names->content;
			return o;
		}
		names = names->next;
	}
	if(s->previous != NULL)
		return getbynamelist(s->previous, names, retname);
	return NULL;
}

// Scope adding
void addclassvardec(SCOPE* s, CLASSVARDEC* v) {
	ensurenoduplicates(s, v->base->names, v->base->debug);
	CLASSVARDEC* new = copy(v, sizeof(CLASSVARDEC));
	new->next = s->classvardecs;
	s->classvardecs = new;
}

void addvardec(SCOPE* s, VARDEC* v) {
	ensurenoduplicates(s, v->names, v->debug);
	VARDEC* new = copy(v, sizeof(VARDEC));
	new->next = s->vardecs;
	s->vardecs = new;
}

void addsubroutdec(SCOPE* s, SUBROUTDEC* sd) {
	ensurenoduplicate(s, sd->name, sd->debug);
	SUBROUTDEC* new = copy(sd, sizeof(SUBROUTDEC));
	new->next = s->subroutines;
	s->subroutines = new;
}

void addclass(SCOPE* s, CLASS* c) {
	ensurenoduplicate(s, c->name, c->debug);
	CLASS* new = copy(c, sizeof(CLASS));
	new->next = s->classes;
	s->classes = new;
}

// Group adding
void addclassvardecs(SCOPE* s, CLASSVARDEC* vs) {
	CLASSVARDEC* next;
	while(vs != NULL) {
		next = vs->next;
		addclassvardec(s, vs);
		vs = next;
	}
}

void addvardecs(SCOPE* s, VARDEC* vs) {
	VARDEC* next;
	while(vs != NULL) {
		next = vs->next;
		addvardec(s, vs);
		vs = next;
	}
}

void addsubroutdecs(SCOPE* s, SUBROUTDEC* ss) {
	SUBROUTDEC* next;
	while(ss != NULL) {
		next = ss->next;
		addsubroutdec(s, ss);
		ss = next;
	}
}

void addclasses(SCOPE* s, CLASS* c) {
	CLASS* next;
	while(c != NULL) {
		next = c->next;
		addclass(s, c);
		c = next;
	}
}
