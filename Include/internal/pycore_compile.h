#pragma once

#include "code.h"
#include "internal/pycore_symtable.h"

struct compiler {
	struct symtable *c_st;
};

typedef struct {
} _PyASTOptimizeState;

#define CALL(FUNC, TYPE, ARG) \
	if (!FUNC((ARG), ctx_, state)) \
		return 0;

#define CALL_SEQ(FUNC, TYPE, ARG) { \
	int i; \
	asdl_ ## TYPE ## _seq *seq = (ARG); /* avoid variable capture */ \
	for (i = 0; i < asdl_seq_LEN(seq); ++i) { \
		TYPE ## _ty elt = (TYPE ## _ty) asdl_seq_GET(seq, i); \
		if (elt != NULL && !FUNC(elt, ctx_, state)) \
			return 0; \
	} \
}

int
compiler_init(struct compiler *c) {
	memset(c, 0, sizeof(struct compiler));
	return 1;
}

int
astfold_expr(expr_ty node_, PyArena *ctx_, _PyASTOptimizeState *state) {
	switch (node_->kind) {
	case Name_kind:
		break;
	case Call_kind:
		CALL(astfold_expr, expr_ty, node_->v.Call.func);
		CALL_SEQ(astfold_expr, expr, node_->v.Call.args);
		// CALL_SEQ(astfold_keyword, keyword, node_->v.Call.keywords);
		break;
	case Constant_kind:
		break;
	default:
		fprintf(stderr, "expr kind %d\n", node_->kind);
		assert(false);
	}
	return 1;
}

int
astfold_stmt(stmt_ty node_, PyArena *ctx_, _PyASTOptimizeState *state) {
	switch (node_->kind) {
	case Expr_kind:
		CALL(astfold_expr, expr_ty, node_->v.Expr.value);
		break;
	default:
		fprintf(stderr, "stmt kind %d\n", node_->kind);
		assert(false);
	}
	return 1;
}

int astfold_body(asdl_stmt_seq *stmts, PyArena *ctx_, _PyASTOptimizeState *state) {
	CALL_SEQ(astfold_stmt, stmt, stmts);	
	return 1;
}

// defined in cpy/Python/ast_opt.c
int
astfold_mod(mod_ty node_, PyArena *ctx_, _PyASTOptimizeState *state) {
	switch (node_->kind) {
	case Module_kind:
		CALL(astfold_body, asdl_seq, node_->v.Module.body);
		break;
	default:
		assert(false);
	}
	return 1;
}

// defined in cpy/Python/ast_opt.c
int
_PyAST_Optimize(mod_ty mod, PyArena *arena, _PyASTOptimizeState *state) {
	// constant folding
	int ret = astfold_mod(mod, arena, state);
	assert(ret);	
	return ret;
}

// Defined in cpy/Python/compile.c
PyCodeObject *
_PyAST_Compile(struct _mod *mod) {
	PyArena *arena = NULL;
	struct compiler c;
	PyCodeObject *co = NULL;
	if (!compiler_init(&c)) {
		return NULL;
	}

	_PyASTOptimizeState state;
	if (!_PyAST_Optimize(mod, arena, &state)) {
		assert(false);
	}
	c.c_st = _PySymtable_Build(mod);
	assert(c.c_st);
  assert(false);
}
