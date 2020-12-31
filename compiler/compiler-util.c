#include "compiler-util.h"

LINE* opvarraw(SCOPE* s, char* op, VAR* v) {
	char* tokens[] = { op, v->memsegment, itoa(v->index) };
	return mksimpleln(tokens, strcount(tokens));
}

LINE* opvar(SCOPE* s, char* op, const char* name) {
	VAR* v = getvar(s, name);
	return opvarraw(s, op, v);
}

LINE* pushvarraw(SCOPE*s, VAR* v) {
	return opvarraw(s, "push", v);
}

LINE* pushvar(SCOPE* s, const char* name) {
	return opvar(s, "push", name);
}

LINE* popvar(SCOPE* s, const char* name) {
	return opvar(s, "pop", name);
}

LINE* poptemp() {
	char* poptemp[] = { "pop", "temp", "0" };
	return mksimpleln(poptemp, strcount(poptemp));
}

LINE* popthatadd() {
	char* popthatadd[] = { "pop", "pointer", "1" };
	return mksimpleln(popthatadd, strcount(popthatadd));
}

LINE* onetoken(char* str) {
	LINE* ln = mkline(1);
	addtoken(ln, ezheapstr(str));
	ln->next = NULL;
	return ln;
}

LINE* mksimpleln(char** tokens, int count) {
	LINE* ln = mkline(count);
	for(int i = 0; i < count; i++)
		addtoken(ln, ezheapstr(tokens[i]));
	ln->next = NULL;
	return ln;
}
