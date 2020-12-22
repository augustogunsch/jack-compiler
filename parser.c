#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parser.h"
#include "parser-constants.h"

char* parseidentifier(PARSER* p);
STATEMENT* parsestatements(PARSER* p);
SUBROUTCALL* parsesubroutcall(PARSER* p);
TERM* parseexpression(PARSER* p);
TERM* parseterm(PARSER* p);

DEBUGINFO* getdebug(PARSER* p) {
	DEBUGINFO* d = (DEBUGINFO*)malloc(sizeof(DEBUGINFO));
	d->file = p->file;
	d->definedat = p->current->definedat;
	return d;
}

void next(PARSER* p) {
	p->current = p->current->next;
}

void checkpoint(PARSER* p) {
	p->checkpoint = p->current;
}

void restorecp(PARSER* p) {
	p->current = p->checkpoint;
}

void unexpectedtoken(PARSER* p) {
	fprintf(stderr, "Unexpected token '%s' (of type %s); line %i, file '%s'\n", p->current->token, tokentypes.items[p->current->type], p->current->definedat, p->file);
}

void unexpected(PARSER* p) {
	unexpectedtoken(p);
	exit(1);
}

void checkcontent(PARSER* p, const char* content) {
	if(strcmp(p->current->token, content))
		unexpected(p);
	next(p);
}

void checktype(PARSER* p, TOKENTYPE type) {
	if(p->current->type != type) {
		fprintf(stderr, "Unexpected %s; line %i, file '%s'\n", tokentypes.items[p->current->type], p->current->definedat, p->file);
		exit(1);
	}
}

TERM* parsetermnullified(PARSER* p) {
	TERM* t = (TERM*)malloc(sizeof(TERM));

	if(p->current->type == integer) {
		t->type = intconstant;
		t->integer = atoi(p->current->token);
		next(p);
	} else if(p->current->type == string) {
		t->type = stringconstant;
		t->string = p->current->token;
		next(p);
	} else if(p->current->type == keyword) {
		t->type = keywordconstant;
		bool valid = false;
		for(int i = 0; i < keywordconstants.size; i++)
			if(!strcmp(p->current->token, keywordconstants.items[i]))
				valid = true;
		if(!valid)
			unexpected(p);

		t->string = p->current->token;
		next(p);
	} else if(!strcmp(p->current->token, "-") || !strcmp(p->current->token, "~")) {
		t->type = unaryopterm;
		next(p);
		t->expression = parseterm(p);
		t->expression->next = NULL;
	} else if(!strcmp(p->current->token, "(")) {
		next(p);
		t->type = innerexpression;
		t->expression = parseexpression(p);
		checkcontent(p, ")");
	} else if(p->current->type == identifier) {
		SUBROUTCALL* call = parsesubroutcall(p);
		if(call == NULL) {
			t->string = p->current->token;
			next(p);
			if(!strcmp(p->current->token, "[")) {
				next(p);
				t->arrayexp = parseexpression(p);
				t->type = arrayitem;
				checkcontent(p, "]");
			} else {
				t->type = varname;
			}
		} else {
			t->type = subroutcall;
			t->call = call;
		}
	} else {
		return NULL;
	}
	return t;
}

bool isop(TOKEN* t) {
	for(int i = 0; i < operators.size; i++)
		if(!strcmp(t->token, operators.items[i]))
			return true;
	return false;
}

TERM* parseexpressionnullified(PARSER* p) {
	TERM* head = parseterm(p);
	TERM* current = head;
	TERM* nextt;
	while(isop(p->current)) {
		current->op = p->current->token[0]; 
		next(p);
		nextt = parseterm(p);
		current->next = nextt;
		current = nextt;
	}
	if(current != NULL)
		current->next = NULL;
	return head;
}

TERM* parseterm(PARSER* p) {
	TERM* t = parsetermnullified(p);
	if(t == NULL)
		unexpected(p);
	return t;
}

TERM* parseexpression(PARSER* p) {
	TERM* t = parseexpressionnullified(p);
	if(t == NULL)
		unexpected(p);
	return t;
}

EXPRESSIONLIST* parseexpressionlist(PARSER* p) {
	if(!strcmp(p->current->token, ")"))
		return NULL;
	EXPRESSIONLIST* head = (EXPRESSIONLIST*)malloc(sizeof(EXPRESSIONLIST));
	head->expression = parseexpressionnullified(p);
	EXPRESSIONLIST* current = head;
	EXPRESSIONLIST* nextls;
	while(!strcmp(p->current->token, ",")) {
		next(p);
		nextls = (EXPRESSIONLIST*)malloc(sizeof(EXPRESSIONLIST));
		nextls->expression = parseexpression(p);
		current->next = nextls;
		current = nextls;
	}
	if(current != NULL)
		current->next = NULL;
	return head;
}

SUBROUTCALL* parsesubroutcall(PARSER* p) {
	checkpoint(p);
	SUBROUTCALL* call = (SUBROUTCALL*)malloc(sizeof(SUBROUTCALL));
	if(!strcmp(p->current->next->token, ".")) {
		if(p->current->type != identifier) {
			free(call);
			return NULL;
		}
		call->parentname = p->current->token;
		next(p);
		next(p);
	}
	else
		call->parentname = NULL;

	if(p->current->type != identifier) {
		free(call);
		restorecp(p);
		return NULL;
	}
	call->debug = getdebug(p);

	call->name = p->current->token;
	next(p);

	if(strcmp(p->current->token, "(")) {
		free(call);
		restorecp(p);
		return NULL;
	}
	next(p);

	call->parameters = parseexpressionlist(p);

	if(strcmp(p->current->token, ")")) {
		free(call);
		restorecp(p);
		return NULL;
	}
	next(p);
	return call;
}

CONDSTATEMENT* parsecond(PARSER* p) {
	checkcontent(p, "(");
	CONDSTATEMENT* st = (CONDSTATEMENT*)malloc(sizeof(CONDSTATEMENT));
	st->expression = parseexpression(p);
	checkcontent(p, ")");
	checkcontent(p, "{");
	st->statements = parsestatements(p);
	checkcontent(p, "}");

	return st;
}

IFSTATEMENT* parseif(PARSER* p) {
	IFSTATEMENT* ifst = (IFSTATEMENT*)malloc(sizeof(IFSTATEMENT));

	ifst->base = parsecond(p);

	if(!strcmp(p->current->token, "else")) {
		next(p);
		checkcontent(p, "{");
		ifst->elsestatements = parsestatements(p);
		checkcontent(p, "}");
	}
	else
		ifst->elsestatements = NULL;

	return ifst;
}

LETSTATEMENT* parselet(PARSER* p) {
	LETSTATEMENT* letstatement = (LETSTATEMENT*)malloc(sizeof(LETSTATEMENT));

	letstatement->varname = parseidentifier(p);
	
	if(!strcmp(p->current->token, "[")) {
		next(p);
		letstatement->arrayind = parseexpression(p);
		checkcontent(p, "]");
	}
	else
		letstatement->arrayind = NULL;

	checkcontent(p, "=");

	letstatement->expression = parseexpression(p);

	checkcontent(p, ";");

	return letstatement;
}

STATEMENT* parsestatement(PARSER* p) {
	STATEMENT* st = (STATEMENT*)malloc(sizeof(STATEMENT));
	if(!strcmp(p->current->token, "let")) {
		next(p);
		st->type = letstatement;
		st->letstatement = parselet(p);
	} else if(!strcmp(p->current->token, "if")) {
		next(p);
		st->type = ifstatement;
		st->ifstatement = parseif(p);
	} else if(!strcmp(p->current->token, "while")) {
		next(p);
		st->type = whilestatement;
		st->whilestatement = parsecond(p);
	} else if(!strcmp(p->current->token, "do")) {
		next(p);
		st->type = dostatement;
		st->dostatement = parsesubroutcall(p);
		checkcontent(p, ";");
	} else if(!strcmp(p->current->token, "return")) {
		next(p);
		st->type = returnstatement;
		if(strcmp(p->current->token, ";")) {
			st->retstatement = parseexpressionnullified(p);
			checkcontent(p, ";");
		}
		else {
			st->retstatement = NULL;
			next(p);
		}
	} else {
		free(st);
		return NULL;
	}
	return st;
}

STATEMENT* parsestatements(PARSER* p) {
	STATEMENT* head = parsestatement(p);
	STATEMENT* current = head;
	STATEMENT* next;
	while(next = parsestatement(p), next != NULL) {
		current->next = next;
		current = next;
	}
	if(current != NULL)
		current->next = NULL;
	return head;
}

char* parsetype(PARSER* p, bool* primitive) {
	char* result = p->current->token;
	if(p->current->type == keyword)
		for(int i = 0; i < vartypes.size; i++) {
			if(!strcmp(p->current->token, vartypes.items[i])) {
				next(p);
				*primitive = true;
				return result;
			}
		}
	else if (p->current->type == identifier) {
		next(p);
		*primitive = false;
		return result;
	}
	else
		unexpected(p);
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

SUBROUTCLASS parsesubroutclass(PARSER* p) {
	return parsepossibilities(p, &subroutclasses);
}

char* parseidentifier(PARSER* p) {
	checktype(p, identifier);
	char* result = p->current->token;
	next(p);
	return result;
}

void parsevardeccommon(PARSER* p, VARDEC* v) {
	v->typeclass = p->current->type;
	v->type = parsetype(p, &(v->primitive));

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
	CLASSVARDEC* current = head;
	CLASSVARDEC* next;
	while(next = parseclassvardec(p), next != NULL) {
		current->next = next;
		current= next;
	}
	if(current != NULL)
		current->next = NULL;
	return head;
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
	VARDEC* current = head;
	VARDEC* next;
	while(next = parsevardec(p), next != NULL) {
		current->next = next;
		current = next;
	}
	if(current != NULL)
		current->next = NULL;
	return head;
}

PARAMETER* parseparameter(PARSER* p) {
	PARAMETER* param = (PARAMETER*)malloc(sizeof(PARAMETER));
	if(!strcmp(p->current->token, ")"))
		return NULL;
	bool dummy;
	param->type = parsetype(p, &dummy);
	param->name = parseidentifier(p);
	return param;
}

PARAMETER* parseparameters(PARSER* p) {
	PARAMETER* head = parseparameter(p);
	PARAMETER* current = head;
	while(!strcmp(p->current->token, ",")) {
		next(p);
		current->next = parseparameter(p);
		current = current->next;
	}
	if(current != NULL)
		current->next = NULL;
	return head;
}

SUBROUTBODY* parsesubroutbody(PARSER* p) {
	SUBROUTBODY* subroutbody = (SUBROUTBODY*)malloc(sizeof(SUBROUTBODY));
	subroutbody->vardecs = parsevardecs(p);
	subroutbody->statements = parsestatements(p);

	return subroutbody;
}

SUBROUTDEC* parsesubroutdec(PARSER* p) {
	SUBROUTCLASS subroutclass = parsesubroutclass(p);
	if(subroutclass == -1)
		return NULL;

	next(p);
	SUBROUTDEC* subroutdec = (SUBROUTDEC*)malloc(sizeof(SUBROUTDEC));
	subroutdec->subroutclass = subroutclass;

	subroutdec->typeclass = p->current->type;
	if(!strcmp(p->current->token, "void")) {
		subroutdec->type = p->current->token;
		next(p);
	}
	else {
		bool dummy;
		subroutdec->type = parsetype(p, &dummy);
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
	SUBROUTDEC* current = head;
	SUBROUTDEC* next;
	while(next = parsesubroutdec(p), next != NULL) {
		current->next = next;
		current = next;
	}
	if(current != NULL)
		current->next = NULL;
	return head;
}

CLASS* parseclass(PARSER* p) {
	checkcontent(p, "class");

	CLASS* class = (CLASS*)malloc(sizeof(CLASS));

	class->debug = getdebug(p);

	class->name = parseidentifier(p);

	checkcontent(p, "{");

	class->vardecs = parseclassvardecs(p);

	class->subroutdecs = parsesubroutdecs(p);

	checkcontent(p, "}");

	class->next = NULL;
	return class;
}

PARSER* mkparser(TOKEN* tokens, char* file) {
	PARSER* parser = (PARSER*)malloc(sizeof(PARSER));
	parser->tokens = tokens;
	parser->current = tokens;
	parser->file = file;
	return parser;
}

void parse(PARSER* parser) {
	parser->output = parseclass(parser);
}
