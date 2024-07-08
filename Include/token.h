#ifndef Py_TOKEN_H
#define Py_TOKEN_H

#define ENDMARKER 0
#define NAME 1
#define STRING 3
#define NEWLINE 4
#define LPAR 7
#define RPAR 8
#define OP 54
#define TYPE_IGNORE 57

const char * const _PyParser_TokenNames[] = {
	[ENDMARKER] = "ENDMARKER",
	[NAME] = "NAME",
	[STRING] = "STRING",
	[NEWLINE] = "NEWLINE",
	[LPAR] = "LPAR",
	[RPAR] = "RPAR",
	[OP] = "OP",
};

const char *_get_token_name(int tok) {
	if (tok >= ARRAY_SIZE(_PyParser_TokenNames) || !_PyParser_TokenNames[tok]) {
		fatal("unknown %d", tok);
	}
	return _PyParser_TokenNames[tok];
}

int
PyToken_OneChar(int c1) {
	switch (c1) {
	case '(': return LPAR;
	case ')': return RPAR;
	}
	fatal("Unhandled OneChar token %d\n", c1);
	return OP;
}

#endif
