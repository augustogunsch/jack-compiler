#ifndef PARSER_INTERNAL_H
#define PARSER_INTERNAL_H
#include <string.h>
#include "parser.h"

/* parser-util
 * Random utilities used in the parser module. */

#define next(parser) parser->current = p->current->next
#define anchorparser(parser) p->checkpoint = p->current
#define rewindparser(parser) p->current = p->checkpoint
#define differs(parser, str) strcmp(parser->current->token, str)
#define nextdiffers(parser, str) strcmp(parser->current->next->token, str)
#define equals(parser, str) !differs(parser, str)
#define nextequals(parser, str) !nextdiffers(parser, str)

void unexpected(PARSER* p);
char* parseidentifier(PARSER* p);
void checkcontent(PARSER* p, const char* content);
DEBUGINFO* getdebug(PARSER* p);
#endif
