#include <stdlib.h>
#include <string.h>
#include "compiler-expressions.h"
#include "compiler-util.h"

/* BEGIN FORWARD DECLARATIONS */

// Miscelaneous
LINE* pushconstant(int n);
LINE* mathopln(char op);
LINE* pushthat();
int countexpressions(EXPRESSIONLIST* explist);
char* toascii(char c);

// Dealing with singular terms
LINEBLOCK* compilestrconst(char* str);
LINEBLOCK* compilekeywordconst(SCOPE* s, TERM* t);
LINEBLOCK* compilearrayitem(SCOPE* s, TERM* t);
LINEBLOCK* compilecallln(SCOPE* s, SUBROUTDEC* d, SUBROUTCALL* call);
LINEBLOCK* pushunaryopterm(SCOPE* s, TERM* t);
LINEBLOCK* compileterm(SCOPE* s, TERM* t);

/* END FORWARD DECLARATIONS */



// Miscelaneous
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
		return mkln(tokens);
	}
	char* tokens[] = { "call", "Math.multiply", "2" };
	return mkln(tokens);
}

LINE* pushconstant(int n) {
	char* tokens[] = { "push", "constant", itoa(n) };
	return mkln(tokens);
}

LINE* pushthat() {
	char* pushthat[] = { "push", "that", "0" };
	return mkln(pushthat);
}

int countexpressions(EXPRESSIONLIST* explist) {
	int i = 0;
	while(explist != NULL) {
		i++;
		explist = explist->next;
	}
	return i;
}

char* toascii(char c) {
	char* result = (char*)malloc(sizeof(char) * (countplaces(c) + 1));
	sprintf(result, "%i", c);
	return result;
}

// Dealing with singular terms
LINEBLOCK* compilestrconst(char* str) {
	if(str[0] == '\0')
		return NULL;

	char* pushchar[] = { "push", "constant", toascii(str[0]) };
	LINEBLOCK* blk = mklnblk(mksimpleln(pushchar, strcount(pushchar)));
	free(pushchar[2]);

	char* appendchar[] = { "call", "String.appendChar", "2" };
	appendln(blk, mkln(appendchar));

	int i = 1;
	char c;
	while(c = str[i], c != '\0') {
		pushchar[2] = toascii(c);
		appendln(blk, mksimpleln(pushchar, strcount(pushchar)));
		free(pushchar[2]);
		appendln(blk, mksimpleln(appendchar, strcount(appendchar)));
		i++;
	}

	char* strsize[] = { "push", "constant", itoa(i) };
	char* mknew[] = { "call", "String.new", "1" };
	appendlnbefore(blk, mkln(mknew));
	appendlnbefore(blk, mkln(strsize));
	free(strsize[2]);

	return blk;
}

LINE* pushthisadd() {
	char* pushthisadd[] = { "push", "pointer", "0" };
	return mkln(pushthisadd);
}

LINE* pushfalse() {
	return pushconstant(0);
}

LINEBLOCK* pushtrue() {
	LINEBLOCK* blk = mklnblk(pushfalse());
	appendln(blk, onetoken("not"));
	return blk;
}

LINEBLOCK* compilekeywordconst(SCOPE* s, TERM* t) {
	if(!strcmp(t->string, "this")) return mklnblk(pushthisadd());
	if(!strcmp(t->string, "false")) return mklnblk(pushfalse());
	if(!strcmp(t->string, "true")) return pushtrue();
	return mklnblk(pushconstant(0));
}

LINEBLOCK* compilearrayitem(SCOPE* s, TERM* t) {
	LINEBLOCK* blk = compileexpression(s, t->array->exp);
	appendln(blk, pushvar(s, t->array->name));

	appendln(blk, onetoken("add"));

	appendln(blk, popthatadd());
	appendln(blk, pushthat());

	return blk;
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
			appendlnbefore(blk, pushthisadd());
		else
			appendlnbefore(blk, pushvarraw(s, v));
	}

	// void functions always return 0
	// therefore must be thrown away
	if(!strcmp(d->type, "void")) {
		appendln(blk, poptemp());
	}

	return blk;
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

LINEBLOCK* compileterm(SCOPE* s, TERM* t) {
	if(t->type == varname) return mklnblk(pushvar(s, t->string));
	if(t->type == intconstant) return mklnblk(pushconstant(t->integer));
	if(t->type == stringconstant) return compilestrconst(t->string);
	if(t->type == keywordconstant) return compilekeywordconst(s, t);
	if(t->type == arrayitem) return compilearrayitem(s, t);
	if(t->type == subroutcall) return compilesubroutcall(s, t->call);
	if(t->type == innerexpression) return compileexpression(s, t->expression);
	return pushunaryopterm(s, t);
}

// Dealing with whole expressions
LINEBLOCK* compileexpression(SCOPE* s, TERM* e) {
	LINEBLOCK* blk = NULL;
	if(e != NULL) {
		while(true) {
			blk = mergelnblks(blk, compileterm(s, e));
			if(e->next != NULL) {
				appendln(blk, mathopln(e->op));
				e = e->next;
			}
			else break;
		}
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
