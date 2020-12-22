#ifndef TOKENIZER_TABLES_H
#define TOKENIZER_TABLES_H
#include "util.h"

const char* keywords[] = {
	"class", "constructor", "function", "method", "field", "static",
	"var", "int", "char", "boolean", "void", "true", "false", "null",
	"this", "let", "do", "if", "else", "while", "return"
};
const int keywordssize = strcount(keyword);

const char* symbols[] = {
	"{", "}", "(", ")", "[", "]", ".", ",", ";", "+", "-", "*", "/",
	"&", "|", "<", ">", "=", "~"
};
const int symbolssize = strcount(symbols);

#endif 
