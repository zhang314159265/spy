#ifndef Py_TOKENIZER_H
#define Py_TOKENIZER_H

#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "pymem.h"
#include "pyport.h"
#include "pymacro.h"
#include "errcode.h"
#include "sutil.h"
#include "token.h"
#include "cpython/fileobject.h"

#define TABSIZE 8

#define is_potential_identifier_start(c) (\
		(c >= 'a' && c <= 'z') \
		|| (c >= 'A' && c <= 'Z') \
		|| c == '_' \
		|| (c >= 128))

#define is_potential_identifier_char(c) (\
		(c >= 'a' && c <= 'z') \
		|| (c >= 'A' && c <= 'Z') \
		|| (c >= '0' && c <= '9') \
		|| c == '_' \
		|| (c >= 128))

#define MAXINDENT 100

struct tok_state {
	FILE *fp;
	char *buf; /* Input buffer, or NULL; malloc'ed if fp != NULL */
	char *cur; /* Next character in buffer */
	char *inp; /* End of data in buffer */
	const char *end; /* End of input buffer if buf != NULL */
	const char *start; /* Start of current token if not NULL */
	int done; /* E_OK normally, E_EOF at EOF, otherwise error code */
	int lineno; /* Current line number */
	int atbol; 

	int indent;
	int indstack[MAXINDENT];
	int pendin;

	int tabsize;
};

struct tok_state *
tok_new(void) {
	struct tok_state *tok = (struct tok_state *)PyMem_Malloc(
			sizeof(struct tok_state));
	if (tok == NULL) {
		return NULL;
	}
	tok->buf = tok->cur = tok->inp = NULL;
	tok->start = NULL;
	tok->end = NULL;
	tok->done = E_OK;
	tok->lineno = 0;
	tok->atbol = 1;

	tok->indent = 0;
	tok->indstack[0] = 0;
	tok->pendin = 0;

	tok->tabsize = TABSIZE;
	return tok;
}

void
PyTokenizer_Free(struct tok_state *tok) {
	assert(0 && "PyTokenizer_Free");
}

struct tok_state *PyTokenizer_FromFile(FILE *fp, const char * enc,
		const char *ps1, const char *ps2) {
	struct tok_state *tok = tok_new();
	if (tok == NULL) {
		return NULL;
	}
	if ((tok->buf = (char *) PyMem_Malloc(BUFSIZ)) == NULL) {
		PyTokenizer_Free(tok);
		return NULL;
	}
	tok->cur = tok->inp = tok->buf;
	tok->end = tok->buf + BUFSIZ;
	tok->fp = fp;
	return tok;
}

int tok_reserve_buf(struct tok_state *tok, Py_ssize_t size) {
	Py_ssize_t cur = tok->cur - tok->buf;
	Py_ssize_t oldsize = tok->inp - tok->buf;
	Py_ssize_t newsize = oldsize + Py_MAX(size, oldsize >> 1);
	if (newsize > tok->end - tok->buf) {
		char *newbuf = tok->buf;
		newbuf = (char *) PyMem_Realloc(newbuf, newsize);
		if (newbuf == NULL) {
			tok->done = E_NOMEM;
			return 0;
		}
		tok->buf = newbuf;
		tok->cur = tok->buf + cur;
		tok->inp = tok->buf + oldsize;
		tok->end = tok->buf + newsize;
	}
	return 1;
}

int tok_readline_raw(struct tok_state *tok) {
	do {
		if (!tok_reserve_buf(tok, BUFSIZ)) {
			return 0;
		}
		char *line = Py_UniversalNewlineFgets(tok->inp,
				(int) (tok->end - tok->inp),
				tok->fp, NULL);
		if (line == NULL) {
			return 1; // no more data
		}
		tok->inp = strchr(tok->inp, '\0');
		if (tok->inp == tok->buf) {
			return 0;
		}
	} while (tok->inp[-1] != '\n');
	return 1;
}

int tok_underflow_file(struct tok_state *tok) {
	if (tok->start == NULL) {
		tok->cur = tok->inp = tok->buf;
	}
	if (!tok_readline_raw(tok)) {
		return 0;
	}
	if (tok->inp == tok->cur) {
		tok->done = E_EOF;
		return 0;
	}
	assert(tok->inp[-1] == '\n');
	tok->lineno++;
	assert(tok->done == E_OK);
	return tok->done == E_OK;
}

int tok_nextc(struct tok_state *tok) {
	int rc;
	for (;;) {
		if (tok->cur != tok->inp) {
			return Py_CHARMASK(*tok->cur++); /* Fast path */
		}
		if (tok->done != E_OK) {
			return EOF;
		}
		rc = tok_underflow_file(tok);
		if (!rc) {
			tok->cur = tok->inp;
			return EOF;
		}
	}
	Py_UNREACHABLE();
}

// Back-up one character
void tok_backup(struct tok_state *tok, int c) {
	if (c != EOF) {
		if (--tok->cur < tok->buf) {
			assert(0);
		}
		if ((int)(unsigned char) *tok->cur != c) {
			assert(0);
		}
	}
}

static int
tok_decimal_tail(struct tok_state *tok) {
	int c;

	while (1) {
		do {
			c = tok_nextc(tok);
		} while (isdigit(c));
		if (c != '_') {
			break;
		}
		c = tok_nextc(tok);
		if (!isdigit(c)) {
			assert(false);
		}
	}
	return c;
}

static int
verify_end_of_number(struct tok_state *tok, int c, const char *kind) {
	// not implemented yet
	return 1;
}

int tok_get(struct tok_state *tok, const char **p_start, const char **p_end) {
	int c;
	int blankline;

	*p_start = *p_end = NULL;

 nextline:
 	blankline = 0;

	if (tok->atbol) {
		int col = 0;
		tok->atbol = 0;
		for (;;) {
			c = tok_nextc(tok);
			if (c == ' ') {
				col++;
			} else if (c == '\t') {
				col = (col / tok->tabsize + 1) * tok->tabsize;
			} else {
				break;
			}
		}
		tok_backup(tok, c);
		if (c == '#' || c == '\n') {
			blankline = 1; /* ignore completely */
		}

		if (!blankline) {
			if (col == tok->indstack[tok->indent]) { 
				// no change
			} else if (col > tok->indstack[tok->indent]) {
				// Indent -- always one
				if (tok->indent + 1 >= MAXINDENT) {
					assert(false);
				}
				tok->pendin++;
				tok->indstack[++tok->indent] = col;
			} else {
				// Dedent -- any number
				while (tok->indent > 0 &&
						col < tok->indstack[tok->indent]) {
					tok->pendin--;
					tok->indent--;
				}
				if (col != tok->indstack[tok->indent]) {
					assert(false);
				}
			}
		}
	}

	tok->start = tok->cur;

	// Return pending indents/dedents
	if (tok->pendin != 0) {
		if (tok->pendin < 0 ) {
			tok->pendin++;
			return DEDENT;
		} else {
			tok->pendin--;
			return INDENT;
		}
	}
 again:
 	tok->start = NULL;
	// Skip spaces
	do {
		c = tok_nextc(tok);
	} while (c == ' ' || c == '\t');

	// set start of current token
	tok->start = tok->cur == NULL ? NULL : tok->cur - 1;

	// skip comment, unless it's a type comment
	if (c == '#') {
		while (c != EOF && c != '\n') {
			c = tok_nextc(tok);
		}
	}

	if (c == EOF) {
		return ENDMARKER;
	}

	// Identifier (most frequent token!)
	if (is_potential_identifier_start(c)) {
		while (is_potential_identifier_char(c)) {
			c = tok_nextc(tok);
		}
		tok_backup(tok, c);
		*p_start = tok->start;
		*p_end = tok->cur;
		return NAME;
	}

	// newline
	if (c == '\n') {
		tok->atbol = 1;
		if (blankline) {
			goto nextline;
		}
		return NEWLINE;
	}

	// period or number starting with period? */
	if (c == '.') {
		c = tok_nextc(tok);
		if (isdigit(c)) {
			assert(false);
		} else if (c == '.') {
			assert(false);
		} else {
			tok_backup(tok, c);
		}
		*p_start = tok->start;
		*p_end = tok->cur;
		return DOT;
	}

	// Number
	if (isdigit(c)) {
		if (c == '0') {
			/* Hex, octal or binary -- maybe. */
			c = tok_nextc(tok);
			if (c == 'x' || c == 'X') {
				assert(false);
			} else if (c == 'o' || c == 'O') {
				assert(false);
			} else if (c == 'b' || c == 'B') {
				assert(false);
			} else {
				int nonzero = 0;
				while (1) {
					if (c == '_') {
						c = tok_nextc(tok);
						if (!isdigit(c)) {
							assert(false);
						}
					}
					if (c != '0') {
						break;
					}
					c = tok_nextc(tok);
				}
				char *zeros_end = tok->cur;
				if (isdigit(c)) {
					nonzero = 1;
					c = tok_decimal_tail(tok);
					if (c == 0) {
						assert(false);
					}
				}
				if (c == '.') {
					assert(false);
				} else if (c == 'e' || c == 'E') {
					assert(false);
				} else if (c == 'j' || c == 'J') {
					assert(false);
				} else if (nonzero) {
					assert(false);
				}
			}
		} else {
			c = tok_decimal_tail(tok);
			if (c == 0) {
				assert(false);
			}
			{
				// accept floating point numbers
				if (c == '.') {
					c = tok_nextc(tok);
				fraction:
					if (isdigit(c)) {
						c = tok_decimal_tail(tok);
						if (c == 0) {
							assert(false);
						}
					}
				}
				if (c == 'e' || c == 'E') {
				exponent:
					c = tok_nextc(tok);
					if (c == '+' || c == '-') {
						c = tok_nextc(tok);
						if (!isdigit(c)) {
							assert(false);
						}
					} else if (!isdigit(c)) {
						assert(false);
					}
					c = tok_decimal_tail(tok);
					if (c == 0) {
						assert(false);
					}
				}
				if (c == 'j' || c == 'J') {
					assert(false);
				} else if (!verify_end_of_number(tok, c, "decimal")) {
					assert(false);
				}
			}
		}
		tok_backup(tok, c);
		*p_start = tok->start;
		*p_end = tok->cur;
		return NUMBER;
	}

	// string
	if (c == '\'' || c == '"') {
		int quote = c;

		// Get rest of string
		while (true) {
			c = tok_nextc(tok);
			assert(c != EOF && c != '\n');
			if (c == quote) {
				break;
			}
		}
		*p_start = tok->start;
		*p_end = tok->cur;
		return STRING;
	}

	// Check for two-character token
	{
		int c2 = tok_nextc(tok);
		int token = PyToken_TwoChars(c, c2);
		if (token != OP) {
			int c3 = tok_nextc(tok);
			int token3 = PyToken_ThreeChars(c, c2, c3);
			if (token3 != OP) {
				token = token3;
			} else {
				tok_backup(tok, c3);
			}

			*p_start = tok->start;
			*p_end = tok->cur;
			return token;
		}
		tok_backup(tok, c2);
	}

	// Punctuation character
	*p_start = tok->start;
	*p_end = tok->cur;
	return PyToken_OneChar(c);
}

int
PyTokenizer_Get(struct tok_state *tok, const char **p_start, const char **p_end) {
	int result = tok_get(tok, p_start, p_end);
	#if 1
	printf("PyTokenizer_Get got: %d ('%c') ", result, (char) result);
	if (p_start) {
		printf("%.*s", (int) (*p_end - *p_start), *p_start);
	}
	printf("\n");
	#endif
	return result;
}

#endif
