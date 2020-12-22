#ifndef PARSER_INTERNAL_H
#define PARSER_INTERNAL_H
#include <string.h>
#include "parser.h"

#define mkstrlist(name, array) STRINGARRAY name = { .items = array, .size = strcount(array) }
#define next(parser) parser->current = p->current->next
#define rewindparser(parser) p->checkpoint = p->current
#define anchorparser(parser) p->current = p->checkpoint
#define differs(parser, str) strcmp(parser->current->token, str)
#define nextdiffers(parser, str) strcmp(parser->current->next->token, str)
#define equals(parser, str) !differs(parser, str)
#define nextequals(parser, str) !nextdiffers(parser, str)

void unexpected(PARSER* p);
char* parseidentifier(PARSER* p);
void checkcontent(PARSER* p, const char* content);
DEBUGINFO* getdebug(PARSER* p);
#endif
