// should be autogenerated

#ifndef Py_TOKEN_H
#define Py_TOKEN_H

#define ENDMARKER 0
#define NAME 1
#define NUMBER 2
#define STRING 3
#define NEWLINE 4
#define INDENT 5
#define DEDENT 6
#define LPAR 7
#define RPAR 8
#define LSQB 9
#define RSQB 10
#define COLON 11
#define COMMA 12
#define PLUS 14
#define MINUS 15
#define STAR 16
#define SLASH 17
#define VBAR 18
#define AMPER 19
#define LESS 20
#define EQUAL 22
#define DOT 23
#define PERCENT 24
#define LBRACE 25
#define RBRACE 26
#define EQEQUAL 27
#define CIRCUMFLEX 32
#define LEFTSHIFT 33
#define RIGHTSHIFT 34
#define DOUBLESTAR 35
#define PLUSEQUAL 36
#define DOUBLESLASH 47
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

// defined in cpy/Parser/token.c
int
PyToken_OneChar(int c1) {
	switch (c1) {
	case '%': return PERCENT;
	case '+': return PLUS;
	case '(': return LPAR;
	case ')': return RPAR;
	case ':': return COLON;
	case '=': return EQUAL;
	case ',': return COMMA;
	case '/': return SLASH;
	case '-': return MINUS;
	case '<': return LESS;
	case '*': return STAR;
	case '[': return LSQB;
	case ']': return RSQB;
	case '{': return LBRACE;
	case '}': return RBRACE;
	case '&': return AMPER;
	case '|': return VBAR;
	case '^': return CIRCUMFLEX;
	}
	fatal("Unhandled OneChar token %d ('%c')\n", c1, (char) c1);
	return OP;
}

int PyToken_TwoChars(int c1, int c2) {
	switch (c1) {
	case '+':
		switch (c2) {
		case '=': return PLUSEQUAL;
		}
		break;
	case '=':
		switch (c2) {
		case '=': return EQEQUAL;
		}
		break;
	case '*':
		switch (c2) {
		case '*': return DOUBLESTAR;
		}
		break;
	case '/':
		switch (c2) {
		case '/': return DOUBLESLASH;
		}
		break;
	case '<':
		switch (c2) {
		case '<': return LEFTSHIFT;
		}
		break;
	case '>':
		switch (c2) {
		case '>': return RIGHTSHIFT;
		}
		break;
	}
	return OP;
}

#endif
