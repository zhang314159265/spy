#pragma once

// The FstringParser is designed to add a mix of strings and
// f-strings, and concat them together as needed. Ultimately, it
// generates an expr_ty.
typedef struct {
	PyObject *last_str;
	int fmode;
} FstringParser;

void
_PyPegen_FstringParser_Init(FstringParser *state) {
	state->last_str = NULL;
	state->fmode = 0;
	// ExprList_Init(&state->expr_list);
}

// Add a non-f-string (that is, a regular literal string). str is
// decref'd
int
_PyPegen_FstringParser_ConcatAndDel(FstringParser *state, PyObject *str) {
	assert(PyUnicode_CheckExact(str));

	if (PyUnicode_GET_LENGTH(str) == 0) {
		assert(false);
		#if 0
		Py_DECREF(str);
		return 0;
		#endif
	}
	if (!state->last_str) {
		// We didn't have a string before, so just remember this one
		state->last_str = str;
	} else {
		assert(false);
	}
	return 0;
}

PyObject *
decode_unicode_with_escapes(Parser *parser, const char *s, size_t len, Token *t) {
	assert(false);
}

int
_PyPegen_parsestr(Parser *p, int *rawmode, PyObject **result, Token *t) {
	const char *s = PyBytes_AsString(t->bytes);
	if (s == NULL) {
		return -1;
	}

	size_t len;
	int quote = Py_CHARMASK(*s);
	// if (Py_ISALPHA(quote)) {
	if (isalpha(quote)) {
		assert(false);
	}
	assert(quote == '\'' || quote == '\"');
	++s; // skip the leading quote char
	len = strlen(s);
	if (s[--len] != quote) {
		assert(false);
	}
	if (len >= 4 && s[0] == quote && s[1] == quote) {
		// A triple quoted string.
		assert(false);
	}

	// Not an f-string
	// Avoid invoking escape decoding routines if possible.
	*rawmode = *rawmode || strchr(s, '\\') == NULL;
	if (*rawmode) {
		*result = PyUnicode_DecodeUTF8Stateful(s, len, NULL, NULL);
	} else {
		*result = decode_unicode_with_escapes(p, s, len, t);
	}
	return *result == NULL ? -1 : 0;
}

expr_ty
make_str_node_and_del(Parser *p, PyObject **str, Token *first_token, Token *last_token) {
	PyObject *s = *str;
	*str = NULL;
	return _PyAST_Constant(s);	
}

expr_ty
_PyPegen_FstringParser_Finish(Parser *p, FstringParser *state, Token* first_token, Token *last_token) {
	if (!state->fmode) {
		if (!state->last_str) {
			assert(false);
		}
		return make_str_node_and_del(p, &state->last_str, first_token, last_token);
	}
	assert(false);
}
