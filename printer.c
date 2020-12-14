#include "printer.h"
void printexpression(TERM* e, FILE* output, int depth);
void printstatements(STATEMENT* st, FILE* output, int depth);

const char* tmpvarclasses[] = {
	"static", "field"
};
const int tmpvarclassessize = sizeof(tmpvarclasses) / sizeof(char*);

const char* tmpsubroutclasses[] = {
	"constructor", "function", "method"
};
const int tmpsubroutclassessize = sizeof(tmpsubroutclasses) / sizeof(char*);

const char* tmptokentypes[] = {
	"keyword", "identifier"
};

void printident(FILE* output, int depth) {
	for(int i = 0; i < depth; i++)
		fprintf(output, "  ");
}

void printstringlist(FILE* output, int depth, STRINGLIST* ls) {
	printident(output, depth);
	fprintf(output, "<identifier> %s </identifier>\r\n", ls->content);
	if(ls->next != NULL) {
		printident(output, depth);
		fprintf(output, "<symbol> , </symbol>\r\n");
		printstringlist(output, depth, ls->next);
	}
}

void printvardec(VARDEC* vd, FILE* output, int depth) {
	printident(output, depth);
	fprintf(output, "<%s> %s </%s>\r\n", tmptokentypes[vd->typeclass], vd->type, tmptokentypes[vd->typeclass]);

	printstringlist(output, depth, vd->names);

	printident(output, depth);
	fprintf(output, "<symbol> ; </symbol>\r\n");
}

void printvardecs(VARDEC* vd, FILE* output, int depth) {
	VARDEC* current = vd;
	while(current != NULL) {
		printident(output, depth);
		fprintf(output, "<varDec>\r\n");

		printident(output, depth+1);
		fprintf(output, "<keyword> var </keyword>\r\n");
		printvardec(current, output, depth+1);
		current = current->next;

		printident(output, depth);
		fprintf(output, "</varDec>\r\n");
	}
}

void printclassvardec(CLASSVARDEC* vd, FILE* output, int depth) {
	printident(output, depth);
	fprintf(output, "<keyword> %s </keyword>\r\n", tmpvarclasses[vd->varclass]);

	printvardec(vd->base, output, depth);
}

void printclassvardecs(CLASSVARDEC* vd, FILE* output, int depth) {
	CLASSVARDEC* current = vd;
	while(current != NULL) {
		printident(output, depth);
		fprintf(output, "<classVarDec>\r\n");

		printclassvardec(current, output, depth+1);
		current = current->next;

		printident(output, depth);
		fprintf(output, "</classVarDec>\r\n");
	}
}

void printparameter(PARAMETER* p, FILE* output, int depth) {
	printident(output, depth);
	fprintf(output, "<keyword> %s </keyword>\r\n", p->type);

	printident(output, depth);
	fprintf(output, "<identifier> %s </identifier>\r\n", p->name);

	if(p->next != NULL) {
		printident(output, depth);
		fprintf(output, "<symbol> , </symbol>\r\n");
	}
}

void printparameters(PARAMETER* p, FILE* output, int depth) {
	printident(output, depth);
	fprintf(output, "<parameterList>\r\n");

	PARAMETER* current = p;
	while(current != NULL) {
		printparameter(current, output, depth+1);
		current = current->next;
	}

	printident(output, depth);
	fprintf(output, "</parameterList>\r\n");
}

void printexpressionlist(EXPRESSIONLIST* list, FILE* output, int depth) {
	printident(output, depth);
	fprintf(output, "<expressionList>\r\n");

	if(list != NULL) {
		EXPRESSIONLIST* current = list;
		while(current != NULL) {
			printexpression(current->expression, output, depth+1);
			current = current->next;
			if(current != NULL) {
				printident(output, depth+1);
				fprintf(output, "<symbol> , </symbol>\r\n");
			}
		}
	}

	printident(output, depth);
	fprintf(output, "</expressionList>\r\n");
}

void printsubroutcall(SUBROUTCALL* c, FILE* output, int depth) {
	if(c->parentname != NULL) {
		printident(output, depth);
		fprintf(output, "<identifier> %s </identifier>\r\n", c->parentname);
		printident(output, depth);
		fprintf(output, "<symbol> . </symbol>\r\n");
	}

	printident(output, depth);
	fprintf(output, "<identifier> %s </identifier>\r\n", c->name);

	printident(output, depth);
	fprintf(output, "<symbol> ( </symbol>\r\n");

	printexpressionlist(c->parameters, output, depth);

	printident(output, depth);
	fprintf(output, "<symbol> ) </symbol>\r\n");
}

void printterm(TERM* e, FILE* output, int depth) {
	printident(output, depth);
	fprintf(output, "<term>\r\n");

	if(e->type == varname) {
		printident(output, depth+1);
		fprintf(output, "<identifier> %s </identifier>\r\n", e->string);
	} else if(e->type == subroutcall) {
		printsubroutcall(e->call, output, depth+1);
	} else if(e->type == stringconstant) {
		printident(output, depth+1);
		fprintf(output, "<stringConstant> %s </stringConstant>\r\n", e->string);
	} else if(e->type == keywordconstant) {
		printident(output, depth+1);
		fprintf(output, "<keyword> %s </keyword>\r\n", e->string);
	} else if(e->type == intconstant) {
		printident(output, depth+1);
		fprintf(output, "<integerConstant> %i </integerConstant>\r\n", e->integer);
	} else if(e->type == arrayitem) {
		printident(output, depth+1);
		fprintf(output, "<identifier> %s </identifier>\r\n", e->string);

		printident(output, depth+1);
		fprintf(output, "<symbol> [ </symbol>\r\n");

		printexpression(e->arrayexp, output, depth+1);

		printident(output, depth+1);
		fprintf(output, "<symbol> ] </symbol>\r\n");
	} else if(e->type == innerexpression) {
		printident(output, depth+1);
		fprintf(output, "<symbol> ( </symbol>\r\n");

		printexpression(e->expression, output, depth+1);

		printident(output, depth+1);
		fprintf(output, "<symbol> ) </symbol>\r\n");
	} else {
		printident(output, depth+1);
		fprintf(output, "<symbol> %c </symbol>\r\n", e->unaryop);

		printterm(e->expression, output, depth+1);
	}

	printident(output, depth);
	fprintf(output, "</term>\r\n");

	if(e->next != NULL) {
		printident(output, depth);
		fprintf(output, "<symbol> %c </symbol>\r\n", e->op);
	}
}

void printexpression(TERM* e, FILE* output, int depth) {
	printident(output, depth);
	fprintf(output, "<expression>\r\n");

	TERM* current = e;
	while(current != NULL) {
		printterm(current, output, depth+1);
		current = current->next;
	}

	printident(output, depth);
	fprintf(output, "</expression>\r\n");
}

void printcond(CONDSTATEMENT* st, FILE* output, int depth) {
	printident(output, depth);
	fprintf(output, "<symbol> ( </symbol>\r\n");

	printexpression(st->expression, output, depth);

	printident(output, depth);
	fprintf(output, "<symbol> ) </symbol>\r\n");

	printident(output, depth);
	fprintf(output, "<symbol> { </symbol>\r\n");

	printstatements(st->statements, output, depth);

	printident(output, depth);
	fprintf(output, "<symbol> } </symbol>\r\n");
}

void printif(IFSTATEMENT* st, FILE* output, int depth) {
	printident(output, depth);
	fprintf(output, "<ifStatement>\r\n");

	printident(output, depth+1);
	fprintf(output, "<keyword> if </keyword>\r\n");

	printcond(st->base, output, depth+1);

	if(st->elsestatements != NULL) {
		printident(output, depth+1);
		fprintf(output, "<keyword> else </keyword>\r\n");

		printident(output, depth+1);
		fprintf(output, "<symbol> { </symbol>\r\n");
		
		printstatements(st->elsestatements, output, depth+1);

		printident(output, depth+1);
		fprintf(output, "<symbol> } </symbol>\r\n");
	}

	printident(output, depth);
	fprintf(output, "</ifStatement>\r\n");
}

void printwhile(CONDSTATEMENT* st, FILE* output, int depth) {
	printident(output, depth);
	fprintf(output, "<whileStatement>\r\n");

	printident(output, depth+1);
	fprintf(output, "<keyword> while </keyword>\r\n");

	printcond(st, output, depth+1);

	printident(output, depth);
	fprintf(output, "</whileStatement>\r\n");
}

void printlet(LETSTATEMENT* st, FILE* output, int depth) {
	printident(output, depth);
	fprintf(output, "<letStatement>\r\n");

	printident(output, depth+1);
	fprintf(output, "<keyword> let </keyword>\r\n");

	printident(output, depth+1);
	fprintf(output, "<identifier> %s </identifier>\r\n", st->varname);

	if(st->arrayind != NULL) {
		printident(output, depth+1);
		fprintf(output, "<symbol> [ </symbol>\r\n");

		printexpression(st->arrayind, output, depth+1);

		printident(output, depth+1);
		fprintf(output, "<symbol> ] </symbol>\r\n");
	}

	printident(output, depth+1);
	fprintf(output, "<symbol> = </symbol>\r\n");

	printexpression(st->expression, output, depth+1);

	printident(output, depth+1);
	fprintf(output, "<symbol> ; </symbol>\r\n");

	printident(output, depth);
	fprintf(output, "</letStatement>\r\n");
}

void printdo(SUBROUTCALL* st, FILE* output, int depth) {
	printident(output, depth);
	fprintf(output, "<doStatement>\r\n");

	printident(output, depth+1);
	fprintf(output, "<keyword> do </keyword>\r\n");

	printsubroutcall(st, output, depth+1);

	printident(output, depth+1);
	fprintf(output, "<symbol> ; </symbol>\r\n");

	printident(output, depth);
	fprintf(output, "</doStatement>\r\n");
}

void printreturn(TERM* st, FILE* output, int depth) {
	printident(output, depth);
	fprintf(output, "<returnStatement>\r\n");

	printident(output, depth+1);
	fprintf(output, "<keyword> return </keyword>\r\n");

	if(st != NULL)
		printexpression(st, output, depth+1);

	printident(output, depth+1);
	fprintf(output, "<symbol> ; </symbol>\r\n");

	printident(output, depth);
	fprintf(output, "</returnStatement>\r\n");
}

void printstatement(STATEMENT* st, FILE* output, int depth) {
	if(st->type == ifstatement)
		printif(st->ifst, output, depth);
	else if(st->type == letstatement)
		printlet(st->letst, output, depth);
	else if(st->type == whilestatement)
		printwhile(st->whilest, output, depth);
	else if(st->type == dostatement)
		printdo(st->dost, output, depth);
	else
		printreturn(st->retst, output, depth);
}

void printstatements(STATEMENT* st, FILE* output, int depth) {
	printident(output, depth);
	fprintf(output, "<statements>\r\n");

	STATEMENT* current = st;
	while(current != NULL) {
		printstatement(current, output, depth+1);
		current = current->next;
	}

	printident(output, depth);
	fprintf(output, "</statements>\r\n");
}

void printsubroutbody(SUBROUTBODY* bd, FILE* output, int depth) {
	printident(output, depth);
	fprintf(output, "<subroutineBody>\r\n");

	printident(output, depth+1);
	fprintf(output, "<symbol> { </symbol>\r\n");

	printvardecs(bd->vardecs, output, depth+1);

	printstatements(bd->statements, output, depth+1);

	printident(output, depth+1);
	fprintf(output, "<symbol> } </symbol>\r\n");

	printident(output, depth);
	fprintf(output, "</subroutineBody>\r\n");
}

void printsubroutdec(SUBDEC* sd, FILE* output, int depth) {
	printident(output, depth);
	fprintf(output, "<keyword> %s </keyword>\r\n", tmpsubroutclasses[sd->subroutclass]);

	printident(output, depth);
	fprintf(output, "<%s> %s </%s>\r\n", tmptokentypes[sd->typeclass], sd->type, tmptokentypes[sd->typeclass]);

	printident(output, depth);
	fprintf(output, "<identifier> %s </identifier>\r\n", sd->name);

	printident(output, depth);
	fprintf(output, "<symbol> ( </symbol>\r\n");

	printparameters(sd->parameters, output, depth);

	printident(output, depth);
	fprintf(output, "<symbol> ) </symbol>\r\n");

	printsubroutbody(sd->body, output, depth);
}

void printsubroutdecs(SUBDEC* sd, FILE* output, int depth) {
	SUBDEC* current = sd;
	while(current != NULL) {
		printident(output, depth);
		fprintf(output, "<subroutineDec>\r\n");

		printsubroutdec(current, output, depth+1);
		current = current->next;

		printident(output, depth);
		fprintf(output, "</subroutineDec>\r\n");
	}
}

void printclass(CLASS* c, FILE* output, int depth) {
	printident(output, depth);
	fprintf(output, "<class>\r\n");

	printident(output, depth+1);
	fprintf(output, "<keyword> class </keyword>\r\n");

	printident(output, depth+1);
	fprintf(output, "<identifier> %s </identifier>\r\n", c->name);

	printident(output, depth+1);
	fprintf(output, "<symbol> { </symbol>\r\n");

	printclassvardecs(c->vardecs, output, depth+1);

	printsubroutdecs(c->subdecs, output, depth+1);

	printident(output, depth+1);
	fprintf(output, "<symbol> } </symbol>\r\n");

	printident(output, depth);
	fprintf(output, "</class>\r\n");
}

void printparser(FILE* output, PARSER* p) {
	printclass(p->output, output, 0);
}
