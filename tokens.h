#ifndef TOKENS_H
#define TOKENS_H

const char* keywords[] = {
	"class", "constructor", "function", "method", "field", "static",
	"var", "int", "char", "boolean", "void", "true", "false", "null",
	"this", "let", "do", "if", "else", "while", "return"
};
const int keywordssize = sizeof(keywords) / sizeof(char*);

const char* symbols[] = {
	"{", "}", "(", ")", "[", "]", ".", ",", ";", "+", "-", "*", "/",
	"&", "|", "<", ">", "=", "~"
};
const int symbolssize = sizeof(symbols) / sizeof(char*);

#endif 
