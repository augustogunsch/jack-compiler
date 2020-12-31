#include <stdlib.h>
#include <string.h>
#include "os.h"
#include "util.h"

CLASS* classes = NULL;

CLASS* addclass(const char* name) {
	CLASS* c = (CLASS*)malloc(sizeof(CLASS));
	c->name = ezheapstr(name);
	c->subroutdecs = NULL;
	c->next = classes;
	classes = c;
	return c;
}

void adddec(CLASS* c, SUBROUTCLASS subroutclass, char* type, const char* name) {
	SUBROUTDEC* dec = (SUBROUTDEC*)malloc(sizeof(SUBROUTDEC));
	dec->class = c;
	dec->subroutclass = subroutclass;
	dec->type = type;
	dec->name = ezheapstr(name);
	dec->next = c->subroutdecs;
	c->subroutdecs = dec;
}

void populatemath() {
	CLASS* mathclass = addclass("Math");
	adddec(mathclass, function, "int", "multiply");
	adddec(mathclass, function, "int", "divide");
	adddec(mathclass, function, "int", "min");
	adddec(mathclass, function, "int", "max");
	adddec(mathclass, function, "int", "sqrt");
}

void populatestringclass() {
	CLASS* strclass = addclass("String");
	adddec(strclass, constructor, "String", "new");
	adddec(strclass, method, "int", "dispose");
	adddec(strclass, method, "int", "length");
	adddec(strclass, method, "char", "charAt");
	adddec(strclass, method, "void", "setCharAt");
	adddec(strclass, method, "String", "appendChar");
	adddec(strclass, method, "void", "eraseLastChar");
	adddec(strclass, method, "int", "intValue");
	adddec(strclass, method, "void", "setInt");
	adddec(strclass, function, "char", "backSpace");
	adddec(strclass, function, "char", "doubleQuote");
	adddec(strclass, function, "char", "newLine");
}

void populatearray() {
	CLASS* arrclass = addclass("Array");
	adddec(arrclass, function, "Array", "new");
	adddec(arrclass, method, "void", "dispose");
}

void populateoutput() {
	CLASS* outclass = addclass("Output");
	adddec(outclass, function, "void", "moveCursor");
	adddec(outclass, function, "void", "printChar");
	adddec(outclass, function, "void", "printString");
	adddec(outclass, function, "void", "printInt");
	adddec(outclass, function, "void", "printLn");
	adddec(outclass, function, "void", "backSpace");
}

void populatescreen() {
	CLASS* scrclass = addclass("Screen");
	adddec(scrclass, function, "void", "clearScreen");
	adddec(scrclass, function, "void", "setColor");
	adddec(scrclass, function, "void", "drawPixel");
	adddec(scrclass, function, "void", "drawLine");
	adddec(scrclass, function, "void", "drawRectangle");
	adddec(scrclass, function, "void", "drawCircle");
}

void populatekeyboard() {
	CLASS* kbdclass = addclass("Keyboard");
	adddec(kbdclass, function, "char", "keyPressed");
	adddec(kbdclass, function, "char", "readChar");
	adddec(kbdclass, function, "String", "readLine");
	adddec(kbdclass, function, "int", "readInt");
}

void populatememory() {
	CLASS* memclass = addclass("Memory");
	adddec(memclass, function, "int", "peek");
	adddec(memclass, function, "void", "poke");
	adddec(memclass, function, "Array", "alloc");
	adddec(memclass, function, "void", "deAlloc");
}

void populatesys() {
	CLASS* sysclass = addclass("Sys");
	adddec(sysclass, function, "void", "halt");
	adddec(sysclass, function, "void", "error");
	adddec(sysclass, function, "void", "wait");
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

void freesubroutdecs(SUBROUTDEC* d) {
	free(d->name);
	free(d->type);
	if(d->next != NULL)
		freesubroutdecs(d->next);
}

void freeclasses(CLASS* c) {
	freesubroutdecs(c->subroutdecs);
	free(c->name);
	if(c->next != NULL)
		freeclasses(c->next);
}

void freeos() {
	freeclasses(classes);
}

SUBROUTDEC* getsubroutdecinclass(CLASS* c, const char* name) {
	SUBROUTDEC* curr = c->subroutdecs;
	while(curr != NULL) {
		if(!strcmp(curr->name, name))
			return curr;
		curr = curr->next;
	}
	return NULL;
}

CLASS* getosclass(const char* name) {
	CLASS* curr = classes;
	while(curr != NULL) {
		if(!strcmp(curr->name, name))
			return curr;
		curr = curr->next;
	}
	return NULL;
}

SUBROUTDEC* getossubroutdec(SUBROUTCALL* call) {
	CLASS* c = getosclass(call->parentname);
	if(c == NULL)
		return NULL;
	return getsubroutdecinclass(c, call->name);
}
