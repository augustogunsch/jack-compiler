#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "compiler-scopes.h"

// Error messages
void doubledeclaration(const char* name, OBJ* o1, OBJ* o2);
void ensurenoduplicate(SCOPE* s, OBJ* o);

// Generic getters
OBJ* getbynamelist(SCOPE* s, STRINGLIST* names, const char** retname);
OBJ* getbynamewithtype(SCOPE* s, const char* name, OBJTYPE type);

// Scope adding
void addclassvardec(SCOPE* s, CLASSVARDEC* v);
void addvardec(SCOPE* s, VARDEC* v);
void addsubroutdec(SCOPE* s, SUBROUTDEC* sd);
void addclass(SCOPE* s, CLASS* c);

// OBJ handling

VARDEC* tovardec(OBJ* obj) {
	if (obj->type == classvardec)
		return ((CLASSVARDEC*)(obj->pointer))->base;
	else if (obj->type == vardec)
		return (VARDEC*)(obj->pointer);
	return NULL;
}

// Error messages
void doubledeclaration(const char* name, OBJ* o1, OBJ* o2) {
	eprintf("Double declaration of '%s' at '%s', line %i; previously defined at '%s', line %i\n",
				name, o1->debug->file, o1->debug->definedat, o2->debug->file, o2->debug->definedat);
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

void ensurenoduplicate(SCOPE* s, OBJ* o) {
	const char* othername;
	OBJ* other = getbynamelist(s, o->names, &othername);
	if(other != NULL)
		doubledeclaration(othername, o, other);
}

// Scope handling

SCOPE* mkscope(SCOPE* prev) {
	SCOPE* s = (SCOPE*)malloc(sizeof(SCOPE));
	s->objects = NULL;
	s->previous = prev;
	s->condlabelcount = 0;
	return s;
}

// Single type getters
SUBROUTDEC* getsubroutdec(SCOPE* s, const char* name) {
	return (SUBROUTDEC*)(getbynamewithtype(s, name, subroutdec)->pointer);
}

CLASS* getclass(SCOPE* s, const char* name) {
	return (CLASS*)(getbynamewithtype(s, name, class)->pointer);
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
		sd = getsubroutdecfromclass(parent->pointer, call->name);
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
OBJ* getbynamelist(SCOPE* s, STRINGLIST* names, const char** retname) {
	OBJ* curr = s->objects;
	while(curr != NULL) {
		STRINGLIST* currn = curr->names;
		while(currn != NULL) {
			STRINGLIST* currattempt = names;
			while(currattempt != NULL) {
				if(!strcmp(currn->content, currattempt->content)) {
					*retname = currn->content;
					return curr;
				}
				currattempt = currattempt->next;
			}
			currn = currn->next;
		}
		curr = curr->next;
	}
	if(s->previous != NULL)
		return getbynamelist(s->previous, names, retname);
	return NULL;
}


OBJ* getbyname(SCOPE* s, const char* name) {
	STRINGLIST* onename = onestr(name);
	const char* dummy;
	return getbynamelist(s, onename, &dummy);
}

OBJ* getbynamewithtype(SCOPE* s, const char* name, OBJTYPE type) {
	OBJ* o = getbyname(s, name);
	if(o->type != type)
		notdeclared(name, o->debug);
	return o;
}

// Scope adding
void addobj(SCOPE* s, OBJ* o) {
	ensurenoduplicate(s, o);
	o->next = s->objects;
	s->objects = o;
}

OBJ* mkobj(void* pointer, OBJTYPE type, DEBUGINFO* debug, STRINGLIST* names) {
	OBJ* o = (OBJ*)malloc(sizeof(OBJ));
	o->pointer = pointer;
	o->type = type;
	o->debug = debug;
	o->names = names;
	return o;
}

void addany(SCOPE* s, void* pointer, OBJTYPE type, DEBUGINFO* debug, STRINGLIST* names) {
	addobj(s, mkobj(pointer, type, debug, names));
}

void addclassvardecs(SCOPE* s, CLASSVARDEC* v) {
	addany(s, v, classvardec, v->base->debug, v->base->names);
	if(v->next != NULL)
		addclassvardecs(s, v->next);
}

void addvardecs(SCOPE* s, VARDEC* v) {
	addany(s, v, vardec, v->debug, v->names);
	if(v->next != NULL)
		addvardecs(s, v->next);
}

void addsubroutdecs(SCOPE* s, SUBROUTDEC* sd) {
	addany(s, sd, subroutdec, sd->debug, onestr(sd->name));
	if(sd->next != NULL)
		addsubroutdecs(s, sd->next);
}

void addclasses(SCOPE* s, CLASS* c) {
	addany(s, c, class, c->debug, onestr(c->name));
	if(c->next != NULL)
		addclasses(s, c->next);
}

void addparameters(SCOPE* s, PARAMETER* p) {
	addany(s, p, parameter, p->debug, onestr(p->name));
	if(p->next != NULL)
		addparameters(s, p->next);
}
