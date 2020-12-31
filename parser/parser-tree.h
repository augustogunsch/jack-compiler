#ifndef PARSER_TREE_H
#define PARSER_TREE_H
#include <stdbool.h>
#include "tokenizer.h"
#include "util.h"

/* parser-tree
 * Type definitions for the parsing tree. */

/* BEGIN FORWARD DECLARATIONS */
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
/* END FORWARD DECLARATIONS */

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
	CLASS* class;
	char* type;
	char* name;
	struct parameter* parameters;
	struct subroutbody* body;
	DEBUGINFO* debug;
	struct subroutdec* next;
} SUBROUTDEC;

typedef struct parameter {
	char* type;
	char* name;
	bool primitive;
	DEBUGINFO* debug;
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
	DEBUGINFO* debug;
	struct vardec* next;
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
	DEBUGINFO* debug;
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

typedef struct {
	char* name;
	struct term* exp;
} ARRAY;

typedef struct term {
	TERMTYPE type;
	union {
		char* string;
		int integer;
		struct subroutcall* call;
		struct term* expression;
		ARRAY* array;
	};
	char op;
	char unaryop;
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
