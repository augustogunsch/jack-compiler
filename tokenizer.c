#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "tokens.h"
#include "tokenizer.h"

typedef enum {
	common, charsymbol, space
} CHARTYPE;

typedef struct {
	char* str;
	int size;
	int count;
} STRING;

TOKEN* mktokenlist() {
	return (TOKEN*)malloc(sizeof(TOKEN));
}

CHARTYPE getchartype(unsigned char c) {
	if(isspace(c)) return space;
	if(isalnum(c) || c == '_' || c == '"') return common;
	return charsymbol;
}

void append(STRING* s, char c) {
	int targsize = sizeof(char) * (s->count + 1);
	if(s->size <= targsize) {
		s->size = targsize * 2;
		s->str = (char*)realloc(s->str, s->size);
	}

	s->str[s->count] = c;
	s->count++;
}

STRING* mkstring(int size) {
	STRING* str = (STRING*)malloc(sizeof(STRING));
	str->size = sizeof(char) * size; // initial size
	str->str = (char*)malloc(str->size);
	str->count = 0;
	return str;
}

bool iskeyword(STRING* tk) {
	for(int i = 0; i < keywordssize; i++)
		if(!strcmp(tk->str, keywords[i]))
			return true;
	return false;
}

bool issymbol(STRING* tk) {
	if(tk->count != 2)
		return false;
	for(int i = 0; i < symbolssize; i++)
		if(!strcmp(tk->str, symbols[i]))
			return true;
	return false;
}

bool isint(char* str) {
	int i = 0;
	while(str[i] != '\0') {
		if(!isdigit(str[i]))
			return false;
		i++;
	}
	return true;
}

bool isintcons(STRING* tk) {
	if(!isint(tk->str))
		return false;
	int val = atoi(tk->str);
	return val >= 0 && val <= 32767;
}

bool isidentifier(STRING* tk) {
	if(isdigit(tk->str[0]))
		return false;

	int count = tk->count - 1;
	for(int i = 0; i < count; i++)
		if(!isalnum(tk->str[i]) && tk->str[i] != '_')
			return false;
	return true;
}

TOKENTYPE gettokentype(STRING* tk, int truen) {
	if(iskeyword(tk)) return keyword;
	if(issymbol(tk)) return symbol;
	if(isintcons(tk)) return integer;
	if(isidentifier(tk)) return identifier;
	fprintf(stderr, "Unexpected token '%s'; line %i\n", tk->str, truen);
	exit(1);
}

TOKEN* appendtokenraw(TOKEN* curitem, STRING* token, int truen, TOKENTYPE type) {
	curitem->token = (char*)malloc(sizeof(char)*token->count);
	strcpy(curitem->token, token->str);
	curitem->truen = truen;
	curitem->type = type;
	TOKEN* nextitem = mktokenlist();
	curitem->next = nextitem;
	token->count = 0;
	return nextitem;
}

TOKEN* appendtoken(TOKEN* curitem, STRING* token, int truen) {
	append(token, '\0');
	return appendtokenraw(curitem, token, truen, gettokentype(token, truen));
}

void skipln(FILE* input) {
	unsigned char c;
	while(c = fgetc(input), c != '\0')
		if(c == '\n')
			break;
}

void skipmultiln(FILE* input, int* lnscount) {
	unsigned char c;
	while(c = fgetc(input), c != '\0')
		if(c == '\n')
			(*lnscount)++;
		else if(c == '*')
			if(fgetc(input) == '/')
				break;
}

bool handlecomment(FILE* input, int* lnscount) {
	unsigned char nextc = fgetc(input);
	if(nextc == '/') {
		skipln(input);
		(*lnscount)++;
		return true;
	}
	else if(nextc == '*') {
		unsigned char furtherc = fgetc(input);
		if(furtherc == '*') {
			skipmultiln(input, lnscount);
			return true;
		}
		ungetc(furtherc, input);
	}
	ungetc(nextc, input);
	return false;
}

void readstr(FILE* input, STRING* tmp, int truen) {
	unsigned char c;
	while(c = fgetc(input), c != '\0') {
		if(c == '\n') {
			fprintf(stderr, "Unexpected end of line; line %i", truen);
			exit(1);
		}
		if(c == '"')
			break;
		append(tmp, c);
	}
	append(tmp, '\0');
}

void freestr(STRING* str) {
	free(str->str);
	free(str);
}

TOKEN* tokenize(FILE* input) {
	TOKEN* head = mktokenlist();
	TOKEN* lastitem = head;
	TOKEN* curitem = head;

	STRING* tmptoken = mkstring(200);
	CHARTYPE lasttype = space;
	CHARTYPE curtype;

	int lnscount = 1;
	
	unsigned char c;
	while(c = fgetc(input), !feof(input)) {
		if(c == '\n') {
			lnscount++;
		}
		else if(c == '/' && handlecomment(input, &lnscount)) 
			continue;
		else if(c == '"') {
			if(lasttype != space)
				curitem = appendtoken(curitem, tmptoken, lnscount);
			readstr(input, tmptoken, lnscount);
			lastitem = curitem;
			curitem = appendtokenraw(curitem, tmptoken, lnscount, string);
			lasttype = space;
			continue;
		}

		curtype = getchartype(c);

		if(curtype == common) {
			if(lasttype == charsymbol) {
				lastitem = curitem;
				curitem = appendtoken(curitem, tmptoken, lnscount);
			}
			append(tmptoken, c);
		} else {
			if(lasttype != space){
				lastitem = curitem;
				curitem = appendtoken(curitem, tmptoken, lnscount);
			}
			if(curtype == charsymbol)
				append(tmptoken, c);
		}
		
		lasttype = curtype;
	}

	lastitem->next = NULL;
	free(curitem);
	freestr(tmptoken);
	fclose(input);
	return head;
}
