#include <stdlib.h>
#include "parser-expressions.h"
#include "parser-internal.h"
#include "parser-statements.h"

STATEMENT* mkstatement(STATEMENTTYPE t);
STATEMENT* parsestatementnullified(PARSER* p);
STATEMENT* parselet(PARSER* p);
CONDSTATEMENT* parsecond(PARSER* p);
STATEMENT* parseif(PARSER* p);
STATEMENT* parsewhile(PARSER* p);
STATEMENT* parsedo(PARSER* p);
STATEMENT* parsereturn(PARSER* p);

STATEMENT* mkstatement(STATEMENTTYPE t) {
	STATEMENT* s = (STATEMENT*)malloc(sizeof(STATEMENT));
	s->type = t;
	return s;
}

// Though nullified, will throw errors if the parsing was on-going
STATEMENT* parsestatementnullified(PARSER* p) {
	if(equals(p, "let"))
		return parselet(p);
	else if(equals(p, "if"))
		return parseif(p);
	else if(equals(p, "while"))
		return parsewhile(p);
	else if(equals(p, "do"))
		return parsedo(p);
	else if(equals(p, "return"))
		return parsereturn(p);
	return NULL;
}

STATEMENT* parsestatements(PARSER* p) {
	STATEMENT* head = parsestatementnullified(p);
	STATEMENT* curr = head;
	STATEMENT* next;
	while(next = parsestatementnullified(p), next != NULL) {
		curr->next = next;
		curr = next;
	}
	if(curr != NULL)
		curr->next = NULL;
	return head;
}

STATEMENT* parselet(PARSER* p) {
	next(p);
	STATEMENT* s = mkstatement(letstatement);
	LETSTATEMENT* letst= (LETSTATEMENT*)malloc(sizeof(LETSTATEMENT));

	letst->varname = parseidentifier(p);
	
	if(equals(p, "[")) {
		next(p);
		letst->arrayind = parseexpression(p);
		checkcontent(p, "]");
	}
	else
		letst->arrayind = NULL;

	checkcontent(p, "=");

	letst->expression = parseexpression(p);

	checkcontent(p, ";");

	s->type = letstatement;
	return s;
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

STATEMENT* parseif(PARSER* p) {
	next(p);
	STATEMENT* s = mkstatement(ifstatement);
	IFSTATEMENT* ifst = (IFSTATEMENT*)malloc(sizeof(IFSTATEMENT));

	ifst->base = parsecond(p);

	if(equals(p, "else")) {
		next(p);
		checkcontent(p, "{");
		ifst->elsestatements = parsestatements(p);
		checkcontent(p, "}");
	}
	else
		ifst->elsestatements = NULL;

	s->type = ifstatement;
	return s;
}

STATEMENT* parsewhile(PARSER* p) {
	next(p);
	STATEMENT* s = mkstatement(whilestatement);

	s->whilestatement = parsecond(p);
	return s;
}

STATEMENT* parsedo(PARSER* p) {
	next(p);
	STATEMENT* s = mkstatement(dostatement);

	s->dostatement = parsesubroutcall(p);

	checkcontent(p, ";");
	return s;
}

STATEMENT* parsereturn(PARSER* p) {
	next(p);
	STATEMENT* s = mkstatement(returnstatement);

	s->retstatement = parseexpressionnullified(p);

	checkcontent(p, ";");
	return s;
}
