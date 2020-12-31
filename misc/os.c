#include <stdlib.h>
#include <string.h>
#include "os.h"
#include "util.h"

#define MATH_COUNT 5
SUBROUTDEC* math[MATH_COUNT];
#define STRING_COUNT 12
SUBROUTDEC* stringclass[STRING_COUNT];
#define ARRAY_COUNT 2
SUBROUTDEC* array[ARRAY_COUNT];
#define OUTPUT_COUNT 6
SUBROUTDEC* output[OUTPUT_COUNT];
#define SCREEN_COUNT 6
SUBROUTDEC* screen[SCREEN_COUNT];
#define KEYBOARD_COUNT 4
SUBROUTDEC* keyboard[KEYBOARD_COUNT];
#define MEMORY_COUNT 4
SUBROUTDEC* memory[MEMORY_COUNT];
#define SYS_COUNT 3
SUBROUTDEC* sys[SYS_COUNT];

CLASS* mkclass(const char* name) {
	CLASS* class = (CLASS*)malloc(sizeof(CLASS));
	class->name = ezheapstr(name);
	return class;
}

SUBROUTDEC* mkdec(const char* class, SUBROUTCLASS subroutclass, char* type, const char* name) {
	SUBROUTDEC* dec = (SUBROUTDEC*)malloc(sizeof(SUBROUTDEC));
	dec->class = mkclass(class);
	dec->subroutclass = subroutclass;
	dec->type = type;
	dec->name = ezheapstr(name);
	return dec;
}

void populatemath() {
	math[0] = mkdec("Math", function, "int", "multiply");
	math[1] = mkdec("Math", function, "int", "divide");
	math[2] = mkdec("Math", function, "int", "min");
	math[3] = mkdec("Math", function, "int", "max");
	math[4] = mkdec("Math", function, "int", "sqrt");
}

void populatestringclass() {
	stringclass[0] = mkdec("String", constructor, "String", "new");
	stringclass[1] = mkdec("String", method, "int", "dispose");
	stringclass[2] = mkdec("String", method, "int", "length");
	stringclass[3] = mkdec("String", method, "char", "charAt");
	stringclass[4] = mkdec("String", method, "void", "setCharAt");
	stringclass[5] = mkdec("String", method, "String", "appendChar");
	stringclass[6] = mkdec("String", method, "void", "eraseLastChar");
	stringclass[7] = mkdec("String", method, "int", "intValue");
	stringclass[8] = mkdec("String", method, "void", "setInt");
	stringclass[9] = mkdec("String", function, "char", "backSpace");
	stringclass[10] = mkdec("String", function, "char", "doubleQuote");
	stringclass[11] = mkdec("String", function, "char", "newLine");
}

void populatearray() {
	array[0] = mkdec("Array", function, "Array", "new");
	array[1] = mkdec("Array", method, "void", "dispose");
}

void populateoutput() {
	output[0] = mkdec("Output", function, "void", "moveCursor");
	output[1] = mkdec("Output", function, "void", "printChar");
	output[2] = mkdec("Output", function, "void", "printString");
	output[3] = mkdec("Output", function, "void", "printInt");
	output[4] = mkdec("Output", function, "void", "printLn");
	output[5] = mkdec("Output", function, "void", "backSpace");
}

void populatescreen() {
	screen[0] = mkdec("Screen", function, "void", "clearScreen");
	screen[1] = mkdec("Screen", function, "void", "setColor");
	screen[2] = mkdec("Screen", function, "void", "drawPixel");
	screen[3] = mkdec("Screen", function, "void", "drawLine");
	screen[4] = mkdec("Screen", function, "void", "drawRectangle");
	screen[5] = mkdec("Screen", function, "void", "drawCircle");
}

void populatekeyboard() {
	keyboard[0] = mkdec("Keyboard", function, "char", "keyPressed");
	keyboard[1] = mkdec("Keyboard", function, "char", "readChar");
	keyboard[2] = mkdec("Keyboard", function, "String", "readLine");
	keyboard[3] = mkdec("Keyboard", function, "int", "readInt");
}

void populatememory() {
	memory[0] = mkdec("Memory", function, "int", "peek");
	memory[1] = mkdec("Memory", function, "void", "poke");
	memory[2] = mkdec("Memory", function, "Array", "alloc");
	memory[3] = mkdec("Memory", function, "void", "deAlloc");
}

void populatesys() {
	sys[0] = mkdec("Sys", function, "void", "halt");
	sys[1] = mkdec("Sys", function, "void", "error");
	sys[2] = mkdec("Sys", function, "void", "wait");
}

void populateos() {
	populatemath();
	populatestringclass();
	populatearray();
	populateoutput();
	populatescreen();
	populatekeyboard();
	populatememory();
	populatesys();
}

void freedecs(SUBROUTDEC** array, int size) {
	for(int i = 0; i < size; i++) {
		free(array[i]->class->name);
		free(array[i]->class);
		free(array[i]->name);
		free(array[i]);
	}
}

void freeos() {
	freedecs(math, MATH_COUNT);
	freedecs(stringclass, STRING_COUNT);
	freedecs(array, ARRAY_COUNT);
	freedecs(output, OUTPUT_COUNT);
	freedecs(screen, SCREEN_COUNT);
	freedecs(keyboard, KEYBOARD_COUNT);
	freedecs(memory, MEMORY_COUNT);
	freedecs(sys, SYS_COUNT);
}

SUBROUTDEC* getdec(SUBROUTDEC** array, int size, const char* name) {
	for(int i = 0; i < size; i++)
		if(!strcmp(array[i]->name, name))
			return array[i];
}

SUBROUTDEC* getossubroutdec(SUBROUTCALL* call) {
	if(!strcmp(call->parentname, "Math"))
		return getdec(math, MATH_COUNT, call->name);
	if(!strcmp(call->parentname, "String"))
		return getdec(stringclass, STRING_COUNT, call->name);
	if(!strcmp(call->parentname, "Array"))
		return getdec(array, ARRAY_COUNT, call->name);
	if(!strcmp(call->parentname, "Output"))
		return getdec(output, OUTPUT_COUNT, call->name);
	if(!strcmp(call->parentname, "Screen"))
		return getdec(screen, SCREEN_COUNT, call->name);
	if(!strcmp(call->parentname, "Keyboard"))
		return getdec(keyboard, KEYBOARD_COUNT, call->name);
	if(!strcmp(call->parentname, "Memory"))
		return getdec(memory, MEMORY_COUNT, call->name);
	if(!strcmp(call->parentname, "Sys"))
		return getdec(sys, SYS_COUNT, call->name);
	return NULL;
}
