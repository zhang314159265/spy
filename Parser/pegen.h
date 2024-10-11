#ifndef PEGEN_H
#define PEGEN_H

#include <assert.h>
#include <stdio.h>
#include "internal/pycore_ast.h"
#include "pyerrors.h"
#include "unicodeobject.h"
#include "pymem.h"
#include "object.h"
#include "bytesobject.h"
#include "Parser/tokenizer.h"

#include "Python/Python-ast.h"

#if 0
// trigger debug messages in Parser/parser.c
#define Py_DEBUG
#define Py_BUILD_CORE
#define Py_DebugFlag 1
#endif

#define UNUSED(expr) (void) (expr)

long PyOS_strtol(const char *, char **, int );
PyObject *PyLong_FromLong(long ival);

typedef struct { operator_ty kind; } AugOperator;

typedef struct _memo {
	int type;  // specify the rule
	void *node;
	int mark;  // the memoized location after matching
	struct _memo *next;
} Memo;

typedef struct {
	int type;
	PyObject *bytes;
	Memo *memo;
} Token;

typedef struct {
	char *str;
	int type;
} KeywordToken;

typedef struct {
	struct tok_state *tok;
	Token **tokens;
	int level;
	int mark;
	int fill, size;
	PyArena *arena;
	int error_indicator;
	KeywordToken **keywords;
	char **soft_keywords;
	int n_keyword_lists;
} Parser;

// Generated function in parse.c - function definition in python.gram
void *_PyPegen_parse(Parser *);
int _PyPegen_fill_token(Parser *p);

void *
_PyPegen_run_parser(Parser *p) {
	void *res = _PyPegen_parse(p);	
	assert(p->level == 0);
	if (res == NULL) {
		assert(false);
	}
	return res;
}

Parser *
_PyPegen_Parser_New(struct tok_state *tok) {
	Parser *p = PyMem_Malloc(sizeof(Parser));
	assert(p);
	p->tok = tok;
	p->level = 0;
	p->mark = 0;
	p->fill = 0;
	p->size = 1;

	p->error_indicator = 0;
	p->arena = NULL; // TODO should use the passed in PyArena
	p->keywords = NULL;
	p->n_keyword_lists = -1;
	p->soft_keywords = NULL;

	p->tokens = PyMem_Malloc(sizeof(Token *));
	assert(p->tokens);
	p->tokens[0] = PyMem_Calloc(1, sizeof(Token));
	assert(p->tokens[0]);
	return p;
}

mod_ty _PyPegen_run_parser_from_file_pointer(FILE *fp) {
	struct tok_state *tok = PyTokenizer_FromFile(fp, NULL, NULL, NULL);
	assert(tok);

	mod_ty result = NULL;
	Parser *p = _PyPegen_Parser_New(tok);;
	assert(p);
	result = _PyPegen_run_parser(p);
	// TODO free parser and tokenizer
	return result;
}

Token *
_PyPegen_expect_token(Parser *p, int type) {
	if (p->mark == p->fill) {
		if (_PyPegen_fill_token(p) < 0) {
			p->error_indicator = 1;
			return NULL;
		}
	}
	Token *t = p->tokens[p->mark];
	if (t->type != type) {
		return NULL;
	}
	p->mark += 1;
	return t;
}

int
_resize_tokens_array(Parser *p) {
	int newsize = p->size * 2;
	Token **new_tokens = PyMem_Realloc(p->tokens, newsize * sizeof(Token *));
	assert(new_tokens);
	p->tokens = new_tokens;
	for (int i = p->size; i < newsize; ++i) {
		p->tokens[i] = PyMem_Calloc(1, sizeof(Token));
		assert(p->tokens[i]);
	}
	p->size = newsize;
	return 0;
}

int
_get_keyword_or_name_type(Parser *p, const char *name, int name_len) {
	assert(name_len > 0);
	if (name_len >= p->n_keyword_lists ||
			p->keywords[name_len] == NULL ||
			p->keywords[name_len]->type == -1) {
		return NAME;
	}
	for (KeywordToken *k = p->keywords[name_len]; k->type != -1; ++k) {
		if (strncmp(k->str, name, name_len) == 0) {
			return k->type;
		}
	}
	return NAME;
}

int
initialize_token(Parser *p, Token *token, const char * start, const char *end, int token_type) {
	assert(token != NULL);

	token->type = (token_type == NAME) ? _get_keyword_or_name_type(p, start, (int) (end - start)) : token_type;
	token->bytes = PyBytes_FromStringAndSize(start, end - start);
	if (token->bytes == NULL) {
		return -1;
	}

	p->fill += 1;

	return 0;
}

int
_PyPegen_fill_token(Parser *p) {
	const char *start;
	const char *end;
	int type = PyTokenizer_Get(p->tok, &start, &end);

	// Record and skip '# type: ignore' comments
	while (type == TYPE_IGNORE) {
		assert(false);
	}

	if (p->fill == p->size && _resize_tokens_array(p) != 0) {
		return -1;
	}
	Token *t = p->tokens[p->fill];
	return initialize_token(p, t, start, end, type);
}

int // bool
_PyPegen_is_memoized(Parser *p, int type, void *pres) {
	if (p->mark == p->fill) {
		if (_PyPegen_fill_token(p) < 0) {
			p->error_indicator = 1;
			return -1;
		}
	}

	Token *t = p->tokens[p->mark];
	for (Memo *m = t->memo; m != NULL; m = m->next) {
		if (m->type == type) {
			p->mark = m->mark;
			*(void **) (pres) = m->node;
			return 1;
		}
	}
	return 0;
}

// Here, mark is the start of the node, while p->mark is the end.
// If node==NULL, they should be the same.
int
_PyPegen_insert_memo(Parser *p, int mark, int type, void *node) {
	// Insert in front
	Memo *m = _PyArena_Malloc(p->arena, sizeof(Memo));
	if (m == NULL) {
		return -1;
	}
	m->type = type;
	m->node = node;
	m->mark = p->mark;
	m->next = p->tokens[mark]->memo;
	p->tokens[mark]->memo = m;
	return 0;
}

int
_PyPegen_update_memo(Parser *p, int mark, int type, void *node) {
	for (Memo *m = p->tokens[mark]->memo; m != NULL; m = m->next) {
		if (m->type == type) {
			// Update existing node.
			m->node = node;
			m->mark = p->mark;
			return 0;
		}
	}
	// Insert new node.
	return _PyPegen_insert_memo(p, mark, type, node);
}

PyObject *
_PyPegen_new_identifier(Parser *p, const char *n) {
	PyObject *id = PyUnicode_DecodeUTF8(n, strlen(n), NULL);
	if (!id) {
		assert(false);
	}
	// PyUnicode_DecodeUTF8 should always return a ready string
	assert(PyUnicode_IS_READY(id));
	if (!PyUnicode_IS_ASCII(id)) {
		assert(false);
	}
	PyUnicode_InternInPlace(&id);
	// _PyArena_AddPyObject
	return id;
}

expr_ty
_PyPegen_name_from_token(Parser *p, Token *t) {
	if (t == NULL) {
		return NULL;
	}
	const char *s = PyBytes_AsString(t->bytes);
	if (!s) {
		p->error_indicator = 1;
		return NULL;
	}
	PyObject *id = _PyPegen_new_identifier(p, s);
	assert(id);
	return _PyAST_Name(id, Load, p->arena);
}

expr_ty
_PyPegen_name_token(Parser *p) {
	Token *t = _PyPegen_expect_token(p, NAME);
	return _PyPegen_name_from_token(p, t);
}

void *_PyPegen_string_token(Parser *p) {
	return _PyPegen_expect_token(p, STRING);
}

// Creates a copy of seq and prepends a to it
asdl_seq *_PyPegen_seq_insert_in_front(Parser *p, void *a, asdl_seq *seq) {
	assert(a);
	if (!seq) {
		assert(false);
	}
	asdl_seq *new_seq = (asdl_seq*) _Py_asdl_generic_seq_new(asdl_seq_LEN(seq) + 1, p->arena);
	if (!new_seq) {
		return NULL;
	}
	asdl_seq_SET_UNTYPED(new_seq, 0, a);
	for (Py_ssize_t i = 1, l = asdl_seq_LEN(new_seq); i < l; ++i) {
		asdl_seq_SET_UNTYPED(new_seq, i, asdl_seq_GET_UNTYPED(seq, i - 1));
	}
	return new_seq;
}

PyObject *
_create_dummy_identifier(Parser *p) {
	return _PyPegen_new_identifier(p, "");
}

// Return dummy NAME
void *
_PyPegen_dummy_name(Parser *p, ...) {
	static void *cache = NULL;

	if (cache != NULL) {
		return cache;
	}

	PyObject *id = _create_dummy_identifier(p);
	if (!id) {
		return NULL;
	}
	cache = _PyAST_Name(id, Load, p->arena);
	return cache;
}

expr_ty _PyPegen_collect_call_seqs(Parser *p, asdl_expr_seq *a, asdl_seq *b) {
	Py_ssize_t args_len = asdl_seq_LEN(a);
	Py_ssize_t total_len = args_len;

	if (b == NULL) {
		return _PyAST_Call(_PyPegen_dummy_name(p), a, NULL);
	}
	assert(false);
}

int
_PyPegen_lookahead_with_int(int positive, Token *(func)(Parser *, int), Parser *p, int arg) {
	int mark = p->mark;
	void *res = func(p, arg);
	p->mark = mark;
	return (res != NULL) == positive;
}

asdl_seq *
_PyPegen_singleton_seq(Parser *p, void *a) {
	assert(a);
	asdl_seq *seq = (asdl_seq*)_Py_asdl_generic_seq_new(1, p->arena);
	if (!seq) {
		return NULL;
	}
	asdl_seq_SET_UNTYPED(seq, 0, a);
	return seq;
}

Py_ssize_t
_get_flattened_seq_size(asdl_seq *seqs) {
	Py_ssize_t size = 0;
	for (Py_ssize_t i = 0, l = asdl_seq_LEN(seqs); i < l; ++i) {
		asdl_seq *inner_seq = asdl_seq_GET_UNTYPED(seqs, i);
		size += asdl_seq_LEN(inner_seq);
	}
	return size;
}

asdl_seq *
_PyPegen_seq_flatten(Parser *p, asdl_seq *seqs) {
	Py_ssize_t flattened_seq_size = _get_flattened_seq_size(seqs);
	assert(flattened_seq_size > 0);

	asdl_seq *flattened_seq = (asdl_seq*)_Py_asdl_generic_seq_new(flattened_seq_size, p->arena);
	if (!flattened_seq) {
		return NULL;
	}

	int flattened_seq_idx = 0;
	for (Py_ssize_t i = 0, l = asdl_seq_LEN(seqs); i < l; ++i) {
		asdl_seq *inner_seq = asdl_seq_GET_UNTYPED(seqs, i);
		for (Py_ssize_t j = 0, li = asdl_seq_LEN(inner_seq); j < li; ++j) {
			asdl_seq_SET_UNTYPED(flattened_seq, flattened_seq_idx++, asdl_seq_GET_UNTYPED(inner_seq, j));
		}
	}
	assert(flattened_seq_idx == flattened_seq_size);
	return flattened_seq;
}

mod_ty
_PyPegen_make_module(Parser *p, asdl_stmt_seq *a) {
	return _PyAST_Module(a, NULL, p->arena);
}

#include "Parser/string_parser.h"

expr_ty
_PyPegen_concatenate_strings(Parser *p, asdl_seq *strings) {
	Py_ssize_t len = asdl_seq_LEN(strings);
	assert(len > 0);

	Token *first = asdl_seq_GET_UNTYPED(strings, 0);
	Token *last = asdl_seq_GET_UNTYPED(strings, len - 1);

	int bytesmode = 0;
	PyObject *bytes_str = NULL;

	FstringParser state;
	_PyPegen_FstringParser_Init(&state);

	for (Py_ssize_t i = 0; i < len; ++i) {
		Token *t = asdl_seq_GET_UNTYPED(strings, i);
		int this_rawmode;
		PyObject *s;
		if (_PyPegen_parsestr(p, &this_rawmode, &s, t) != 0) {
			assert(false);
		}
		// This is a regular string. Concatenate it
		if (_PyPegen_FstringParser_ConcatAndDel(&state, s) < 0) {
			assert(false);
		}
	}

	return _PyPegen_FstringParser_Finish(p, &state, first, last);
}

arg_ty
_PyPegen_add_type_comment_to_arg(Parser *p, arg_ty a, Token *tc) {
	if (tc == NULL) {
		return a;
	}
	assert(false);
}

static int
_make_posargs(Parser *p,
		asdl_arg_seq *plain_names,
		asdl_seq *names_with_default,
		asdl_arg_seq **posargs) {
	assert(plain_names);
	assert(!names_with_default);

	*posargs = plain_names;
	return *posargs == NULL ? -1 : 0;
}

typedef void *SlashWithDefault;
typedef void *StarEtc;

arguments_ty
_PyPegen_make_arguments(Parser *p, asdl_arg_seq *slash_without_default,
		SlashWithDefault *slash_with_default, asdl_arg_seq *plain_names,
		asdl_seq *names_with_default, StarEtc *star_etc) {
	
	asdl_arg_seq *posargs;
	if (_make_posargs(p, plain_names, NULL, &posargs) == -1) {
		assert(false);
		return NULL;
	}
	return _PyAST_arguments(posargs);
}

static PyObject *
parsenumber_raw(const char *s) {
	const char *end;
	long x;
	int imflag;

	assert(s != NULL);
	end = s + strlen(s) - 1;

	imflag = *end == 'j' || *end == 'J';
	if (s[0] == '0') {
		x = PyOS_strtol(s, (char **) &end, 0);
	} else {
		x = PyOS_strtol(s, (char **) &end, 0);
	}
	if (*end == '\0') {
		return PyLong_FromLong(x);
	}
	assert(false);
}

static PyObject *
parsenumber(const char *s) {
	assert(s != NULL);

	if (strchr(s, '_') == NULL) {
		return parsenumber_raw(s);
	}
	assert(false);
}

expr_ty
_PyPegen_number_token(Parser *p) {
	Token *t = _PyPegen_expect_token(p, NUMBER);
	if (t == NULL) {
		return NULL;
	}

	const char *num_raw = PyBytes_AsString(t->bytes);
	if (num_raw == NULL) {
		assert(false);
	}

	PyObject *c = parsenumber(num_raw);

	if (c == NULL) {
		assert(false);
	}

	return _PyAST_Constant(c);
}

static expr_ty
_set_name_context(Parser *p, expr_ty e, expr_context_ty ctx) {
	return _PyAST_Name(e->v.Name.id, ctx, p->arena);
}

expr_ty
_PyPegen_set_expr_context(Parser *p, expr_ty expr, expr_context_ty ctx) {
	assert(expr != NULL);

	expr_ty new = NULL;
	switch (expr->kind) {
	case Name_kind:
		new = _set_name_context(p, expr, ctx);
		break;
	default:
		assert(false);
	}
	return new;
}

AugOperator *
_PyPegen_augoperator(Parser *p, operator_ty kind) {
	AugOperator *a = malloc(sizeof(AugOperator));
	if (!a) {
		return NULL;
	}
	a->kind = kind;
	return a;
}

Token *
_PyPegen_expect_forced_token(Parser *p, int type, const char *expected) {
	assert(false);
}

typedef struct {
	cmpop_ty cmpop;
	expr_ty expr;
} CmpopExprPair;

CmpopExprPair *
_PyPegen_cmpop_expr_pair(Parser *p, cmpop_ty cmpop, expr_ty expr) {
	assert(expr != NULL);
	CmpopExprPair *a = malloc(sizeof(CmpopExprPair));
	if (!a) {
		return NULL;
	}
	a->cmpop = cmpop;
	a->expr = expr;
	return a;
}

void *
CHECK_CALL(Parser *p, void *result) {
	if (result == NULL) {
		assert(false);
	}
	return result;
}

#define CHECK(type, result) ((type) CHECK_CALL(p, result))

asdl_int_seq *
_PyPegen_get_cmpops(Parser *p, asdl_seq *seq) {
	Py_ssize_t len = asdl_seq_LEN(seq);
	assert(len > 0);

	asdl_int_seq *new_seq = _Py_asdl_int_seq_new(len, p->arena);
	if (!new_seq) {
		return NULL;
	}
	for (Py_ssize_t i = 0; i < len; i++) {
		CmpopExprPair *pair = asdl_seq_GET_UNTYPED(seq, i);
		asdl_seq_SET(new_seq, i, pair->cmpop);
	}
	return new_seq;
}

asdl_expr_seq *
_PyPegen_get_exprs(Parser *p, asdl_seq *seq) {
	Py_ssize_t len = asdl_seq_LEN(seq);
	assert(len > 0);

	asdl_expr_seq *new_seq = _Py_asdl_expr_seq_new(len, p->arena);
	if (!new_seq) {
		return NULL;
	}
	for (Py_ssize_t i = 0; i < len; i++) {
		CmpopExprPair *pair = asdl_seq_GET_UNTYPED(seq, i);
		asdl_seq_SET(new_seq, i, pair->expr);
	}
	return new_seq;
}

#endif
