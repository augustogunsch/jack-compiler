#ifndef PARSER_TREE_H
#define PARSER_TREE_H
#include <stdbool.h>
#include "tokenizer.h"
#include "util.h"

// Forward declarations
struct classvardec;
struct parameter;
struct subroutbody;
struct subroutdec;
struct vardec;
struct letstatement;
struct ifstatement;
struct condstatement;
struct subroutcall;
struct term;
struct expressionlist;

// Misc
typedef struct {
	char* file;
	int definedat;
} DEBUGINFO;

// Program structure

typedef struct class {
	char* name;
	struct classvardec* vardecs;
	struct subroutdec* subroutdecs;
	DEBUGINFO* debug;
	struct class* next;
} CLASS;

typedef enum {
	stat, field
} CLASSVARTYPE;

typedef struct classvardec {
	CLASSVARTYPE type;
	struct vardec* base;
	struct classvardec* next;
} CLASSVARDEC;

typedef enum {
	constructor, function, method
} SUBROUTCLASS;

typedef struct subroutdec {
	SUBROUTCLASS subroutclass;
	char* type;
	TOKENTYPE typeclass;
	char* name;
	struct parameter* parameters;
	struct subroutbody* body;
	DEBUGINFO* debug;
	struct subroutdec* next;
} SUBROUTDEC;

typedef struct parameter {
	char* type;
	char* name;
	struct parameter* next;
} PARAMETER;

typedef struct subroutbody {
	struct vardec* vardecs;
	struct statement* statements;
} SUBROUTBODY;

typedef struct vardec {
	char* type;
	bool primitive;
	TOKENTYPE typeclass;
	STRINGLIST* names;
	struct vardec* next;
	DEBUGINFO* debug;
} VARDEC;

// Statements
typedef enum {
	ifstatement, whilestatement, letstatement, dostatement, returnstatement
} STATEMENTTYPE;

typedef struct statement {
	STATEMENTTYPE type;
	union {
		struct letstatement* letstatement;
		struct ifstatement* ifstatement;
		struct condstatement* whilestatement;
		struct subroutcall* dostatement;
		struct term* retstatement;
	};
	struct statement* next;
} STATEMENT;

typedef struct letstatement {
	char* varname;
	struct term* arrayind;
	struct term* expression;
} LETSTATEMENT;

typedef struct ifstatement {
	struct condstatement* base;
	struct statement* elsestatements;
} IFSTATEMENT;

typedef struct condstatement {
	struct term* expression;
	struct statement* statements;
} CONDSTATEMENT;

// Expressions

typedef enum {
	varname, intconstant, stringconstant, keywordconstant, arrayitem, subroutcall, innerexpression, unaryopterm
} TERMTYPE;

typedef struct term {
	TERMTYPE type;
	union {
		char* string;
		int integer;
		struct subroutcall* call;
		struct term* expression;
	};
	struct term* arrayexp;
	char op;
	struct term* next;
} TERM; 

typedef struct subroutcall {
	char* parentname;
	char* name;
	struct expressionlist* parameters;
	DEBUGINFO* debug;
} SUBROUTCALL;

typedef struct expressionlist {
	TERM* expression;
	struct expressionlist* next;
} EXPRESSIONLIST;

#endif
