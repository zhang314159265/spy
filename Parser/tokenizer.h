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

struct tok_state {
	FILE *fp;
	char *buf; /* Input buffer, or NULL; malloc'ed if fp != NULL */
	char *cur; /* Next character in buffer */
	char *inp; /* End of data in buffer */
	const char *end; /* End of input buffer if buf != NULL */
	const char *start; /* Start of current token if not NULL */
	int done; /* E_OK normally, E_EOF at EOF, otherwise error code */
	int lineno; /* Current line number */
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

int tok_get(struct tok_state *tok, const char **p_start, const char **p_end) {
	int c;

	*p_start = *p_end = NULL;

 again:
	// Skip spaces
	do {
		c = tok_nextc(tok);
	} while (c == ' ' || c == '\t');

	// set start of current token
	tok->start = tok->cur == NULL ? NULL : tok->cur - 1;

	// skip comment, unless it's a type comment
	if (c == '#') {
		assert(false);
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
		return NEWLINE;
	}

	// period or number starting with period? */
	if (c == '.') {
		assert(false && ".");
	}

	// Number
	if (isdigit(c)) {
		assert(false && "digit");
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
	}

	// Punctuation character
	return PyToken_OneChar(c);
}

int
PyTokenizer_Get(struct tok_state *tok, const char **p_start, const char **p_end) {
	int result = tok_get(tok, p_start, p_end);
	return result;
}

#endif
