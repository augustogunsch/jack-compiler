#ifndef VM_LINES_H
#define VM_LINES_H
#include <stdio.h>

/* vm-lines
 * Unified standard for the compiler's output and vm-translator's input.
 * It is also used by vm-parser when reading .vm files. */

// Data types
typedef struct line {
	char** tokens;
	int count;
	struct line* next;
} LINE;

typedef struct {
	LINE* head;
	LINE* tail;
} LINEBLOCK;

// Line manipulation
LINE* mkline(int size);
void addtoken(LINE* ln, char* token);

// Line printing
void println(LINE* ln, FILE* stream);
void printlns(LINE* lns, FILE* stream);

// Line freeing
void freeln(LINE* ln);
void freelns(LINE* lns);

// Line block manipulation
LINEBLOCK* mklnblk(LINE* start);
LINEBLOCK* mergelnblks(LINEBLOCK* head, LINEBLOCK* tail);
void appendln(LINEBLOCK* lnblk, LINE* ln);
void appendlnbefore(LINEBLOCK* lnblk, LINE* ln);
#endif
