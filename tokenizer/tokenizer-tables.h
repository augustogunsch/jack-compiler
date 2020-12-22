#ifndef TOKENIZER_TABLES_H
#define TOKENIZER_TABLES_H
#include "util.h"


const char* keywordsraw[] = {
	"class", "constructor", "function", "method", "field", "static",
	"var", "int", "char", "boolean", "void", "true", "false", "null",
	"this", "let", "do", "if", "else", "while", "return"
};
mkstrlist(keywords, keywordsraw);

const char* symbolsraw[] = {
	"{", "}", "(", ")", "[", "]", ".", ",", ";", "+", "-", "*", "/",
	"&", "|", "<", ">", "=", "~"
};
mkstrlist(symbols, symbolsraw);

#endif 
