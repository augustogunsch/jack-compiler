#include <stdlib.h>
#include "parser-internal.h"
#include "parser-structure.h"
#include "parser-statements.h"

CLASS* parseclass(PARSER* p);
int parsepossibilities(PARSER* p, STRINGARRAY* poss);
CLASSVARTYPE parseclassvartype(PARSER* p);
CLASSVARDEC* parseclassvardec(PARSER* p);
CLASSVARDEC* parseclassvardecs(PARSER* p);
SUBROUTCLASS parsesubroutclass(PARSER* p);
SUBROUTDEC* parsesubroutdec(PARSER* p);
SUBROUTDEC* parsesubroutdecs(PARSER* p);
PARAMETER* parseparameter(PARSER* p);
PARAMETER* parseparameters(PARSER* p);
SUBROUTBODY* parsesubroutbody(PARSER* p);
bool isprimitive(TOKEN* tk);
char* parsetype(PARSER* p);
void parsevardeccommon(PARSER* p, VARDEC* v);
VARDEC* parsevardec(PARSER* p);
VARDEC* parsevardecs(PARSER* p);

const char* classvartypesarr[] = { "static", "field" };
const char* vartypesarr[] = { "int", "char", "boolean" };
const char* subroutclassesarr[] = { "constructor", "function", "method" };
mkstrlist(classvartypes, classvartypesarr);
mkstrlist(vartypes, vartypesarr);
mkstrlist(subroutclasses, subroutclassesarr);

CLASS* parseclass(PARSER* p) {
	checkcontent(p, "class");

	CLASS* class = (CLASS*)malloc(sizeof(CLASS));

	class->debug = getdebug(p);

	class->name = parseidentifier(p);

	checkcontent(p, "{");

	class->vardecs = parseclassvardecs(p);

	class->subroutdecs = parsesubroutdecs(p);

	checkcontent(p, "}");
	return class;
}

CLASS* parseclasses(PARSER* p) {
	CLASS* head = parseclass(p);
	CLASS* curr = head;
	while(p->current != NULL && equals(p, "class")) {
		curr->next = parseclass(p);
		curr = curr->next;
	}
	if(curr != NULL)
		curr->next = NULL;
	return head;
}

int parsepossibilities(PARSER* p, STRINGARRAY* poss) {
	for(int i = 0; i < poss->size; i++)
		if(!strcmp(p->current->token, poss->items[i]))
			return i;
	return -1;
}

CLASSVARTYPE parseclassvartype(PARSER* p) {
	return parsepossibilities(p, &classvartypes);
}

CLASSVARDEC* parseclassvardec(PARSER* p) {
	CLASSVARTYPE classvartype = parseclassvartype(p);
	if(classvartype == -1)
		return NULL;
	next(p);

	CLASSVARDEC* classvardec = (CLASSVARDEC*)malloc(sizeof(CLASSVARDEC));
	classvardec->type = classvartype;

	classvardec->base = (VARDEC*)malloc(sizeof(VARDEC));

	parsevardeccommon(p, classvardec->base);

	return classvardec;
}

CLASSVARDEC* parseclassvardecs(PARSER* p) {
	CLASSVARDEC* head = parseclassvardec(p);
	CLASSVARDEC* curr = head;
	CLASSVARDEC* next;
	while(next = parseclassvardec(p), next != NULL) {
		curr->next = next;
		curr= next;
	}
	if(curr != NULL)
		curr->next = NULL;
	return head;
}

SUBROUTCLASS parsesubroutclass(PARSER* p) {
	return parsepossibilities(p, &subroutclasses);
}

SUBROUTDEC* parsesubroutdec(PARSER* p) {
	SUBROUTCLASS subroutclass = parsesubroutclass(p);
	if(subroutclass == -1)
		return NULL;

	next(p);
	SUBROUTDEC* subroutdec = (SUBROUTDEC*)malloc(sizeof(SUBROUTDEC));
	subroutdec->subroutclass = subroutclass;

	if(differs(p, "void"))
		subroutdec->type = parsetype(p);
	else {
		subroutdec->type = p->current->token;
		next(p);
	}

	subroutdec->debug = getdebug(p);

	subroutdec->name = parseidentifier(p);

	checkcontent(p, "(");
	subroutdec->parameters = parseparameters(p);
	checkcontent(p, ")");

	checkcontent(p, "{");
	subroutdec->body = parsesubroutbody(p);
	checkcontent(p, "}");

	return subroutdec;
}

SUBROUTDEC* parsesubroutdecs(PARSER* p) {
	SUBROUTDEC* head = parsesubroutdec(p);
	SUBROUTDEC* curr = head;
	SUBROUTDEC* next;
	while(next = parsesubroutdec(p), next != NULL) {
		curr->next = next;
		curr = next;
	}
	if(curr != NULL)
		curr->next = NULL;
	return head;
}

PARAMETER* parseparameter(PARSER* p) {
	PARAMETER* param = (PARAMETER*)malloc(sizeof(PARAMETER));
	if(equals(p, ")"))
		return NULL;
	param->type = parsetype(p);
	param->name = parseidentifier(p);
	return param;
}

PARAMETER* parseparameters(PARSER* p) {
	PARAMETER* head = parseparameter(p);
	PARAMETER* curr = head;
	while(equals(p, ",")) {
		next(p);
		curr->next = parseparameter(p);
		curr = curr->next;
	}
	if(curr != NULL)
		curr->next = NULL;
	return head;
}

SUBROUTBODY* parsesubroutbody(PARSER* p) {
	SUBROUTBODY* subroutbody = (SUBROUTBODY*)malloc(sizeof(SUBROUTBODY));
	subroutbody->vardecs = parsevardecs(p);
	subroutbody->statements = parsestatements(p);

	return subroutbody;
}

bool isprimitive(TOKEN* tk) {
	if(tk->type == keyword)
		if(existsinarray(&vartypes, tk->token))
			return true;
	return false;
}

char* parsetype(PARSER* p) {
	if(p->current->type != identifier)
		unexpected(p);

	char* result = p->current->token;
	next(p);
	return result;
}

void parsevardeccommon(PARSER* p, VARDEC* v) {
	v->typeclass = p->current->type;
	v->primitive = isprimitive(p->current);
	v->type = parsetype(p);

	STRINGLIST* currstr = (STRINGLIST*)malloc(sizeof(STRINGLIST));
	v->names = currstr;

	v->debug = getdebug(p);

	v->names->content = parseidentifier(p);

	while(!strcmp(p->current->token, ",")) {
		next(p);
		STRINGLIST* nextstr = (STRINGLIST*)malloc(sizeof(STRINGLIST));
		nextstr->content = parseidentifier(p);
		currstr->next = nextstr;
		currstr = nextstr;
	}
	currstr->next = NULL;

	checkcontent(p, ";");
}


VARDEC* parsevardec(PARSER* p) {
	if(strcmp(p->current->token, "var"))
		return NULL;
	next(p);

	VARDEC* vardec = (VARDEC*)malloc(sizeof(VARDEC));

	parsevardeccommon(p, vardec);

	return vardec;
}

VARDEC* parsevardecs(PARSER* p) {
	VARDEC* head = parsevardec(p);
	VARDEC* curr = head;
	VARDEC* next;
	while(next = parsevardec(p), next != NULL) {
		curr->next = next;
		curr = next;
	}
	if(curr != NULL)
		curr->next = NULL;
	return head;
}
