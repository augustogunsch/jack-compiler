#ifndef PARSER_H
#define PARSER_H
#include <stdbool.h>
#include "tokenizer.h"
#include "util.h"

struct statement;
struct explist;

typedef struct {
	char* file;
	int definedat;
} DEBUGINFO;

typedef enum {
	ifstatement, whilestatement, letstatement, dostatement, returnstatement
} STATEMENTTYPE;

typedef enum {
	varname, intconstant, stringconstant, keywordconstant, arrayitem, subroutcall, innerexpression, unaryopterm
} TERMTYPE;

typedef struct {
	char* parentname;
	char* name;
	struct explist* parameters;
	DEBUGINFO* debug;
} SUBROUTCALL;

typedef struct term {
	TERMTYPE type;
	union {
		char* string;
		int integer;
		SUBROUTCALL* call;
		struct term* expression;
	};
	struct term* arrayexp;
	char op;
	struct term* next;
} TERM; 

typedef struct explist {
	TERM* expression;
	struct explist* next;
} EXPRESSIONLIST;

typedef struct {
	TERM* expression;
	struct statement* statements;
} CONDSTATEMENT;

typedef struct {
	CONDSTATEMENT* base;
	struct statement* elsestatements;
} IFSTATEMENT;

typedef struct {
	char* varname;
	TERM* arrayind;
	TERM* expression;
} LETSTATEMENT;

typedef struct statement {
	STATEMENTTYPE type;
	union {
		CONDSTATEMENT* whilest;
		IFSTATEMENT* ifst;
		LETSTATEMENT* letst;
		SUBROUTCALL* dost;
		TERM* retst;
	};
	struct statement* next;
} STATEMENT;

typedef enum {
	stat, field
} VARCLASS;

typedef struct vardec {
	char* type;
	bool primitive;
	TOKENTYPE typeclass;
	STRINGLIST* names;
	struct vardec* next;
	DEBUGINFO* debug;
} VARDEC;

typedef struct classvardec {
	VARCLASS varclass;
	VARDEC* base;
	struct classvardec* next;
} CLASSVARDEC;

typedef enum {
	constructor, function, method
} SUBROUTCLASS;

typedef struct parameter {
	char* type;
	char* name;
	struct parameter* next;
} PARAMETER;

typedef struct SUBROUTBODY {
	VARDEC* vardecs;
	STATEMENT* statements;
} SUBROUTBODY;

typedef struct subdec {
	SUBROUTCLASS subroutclass;
	char* type;
	TOKENTYPE typeclass;
	char* name;
	PARAMETER* parameters;
	SUBROUTBODY* body;
	DEBUGINFO* debug;
	struct subdec* next;
} SUBDEC;

typedef struct cl {
	char* name;
	CLASSVARDEC* vardecs;
	SUBDEC* subdecs;
	DEBUGINFO* debug;
	struct cl* next;
} CLASS;

typedef struct {
	TOKEN* tokens;
	TOKEN* current;
	TOKEN* checkpoint;
	char* file;
	CLASS* output;
} PARSER;

PARSER* mkparser(TOKEN* tokens, char* file);
void parse(PARSER* parser);
#endif
