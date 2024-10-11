// TODO generate this file rather than manually create it
#ifndef Py_INTERNAL_AST_H
#define Py_INTERNAL_AST_H

#include "internal/pycore_asdl.h"

typedef enum _operator {
	Add=1,
} operator_ty;

typedef enum _expr_context { Load=1, Store=2, Del=3} expr_context_ty;

typedef struct _mod *mod_ty;

typedef struct _expr *expr_ty;

typedef struct _stmt *stmt_ty;

typedef struct _arguments *arguments_ty;

typedef struct _arg *arg_ty;

typedef struct {
	_ASDL_SEQ_HEAD
	arg_ty typed_elements[1];
} asdl_arg_seq;

struct _arg {
	identifier arg;
};

struct _arguments {
	asdl_arg_seq *args;
};

typedef struct {
	_ASDL_SEQ_HEAD
	expr_ty typed_elements[1];
} asdl_expr_seq;

typedef struct {
	_ASDL_SEQ_HEAD
	stmt_ty typed_elements[1];
} asdl_stmt_seq;

struct _keyword {
	identifier arg;
	expr_ty value;
};

typedef struct _keyword *keyword_ty;

typedef struct {
	_ASDL_SEQ_HEAD
	keyword_ty typed_elements[1];
} asdl_keyword_seq;

enum _mod_kind {
	Module_kind = 1
};

struct _mod {
	enum _mod_kind kind;
	union {
		struct {
			asdl_stmt_seq *body;
		} Module;
	} v;
};

enum _expr_kind {
	BinOp_kind = 3,
	Call_kind=17,
	Constant_kind=20,
	Attribute_kind=21,
	Starred_kind=23,
	Name_kind=24,
};

struct _expr {
	enum _expr_kind kind;
	union {
		struct {
			expr_ty value;
			identifier attr;
			expr_context_ty ctx;
		} Attribute;

		struct {
			expr_ty left;
			operator_ty op;
			expr_ty right;
		} BinOp;

		struct {
			identifier id;
			expr_context_ty ctx;
		} Name;

		struct {
			expr_ty func;
			asdl_expr_seq *args;
			asdl_keyword_seq *keywords;
		} Call;

		struct {
			constant value;
		} Constant;
	} v;
};

enum _stmt_kind {
	FunctionDef_kind = 1,
	Return_kind = 4,
	Assign_kind = 6,
	AugAssign_kind = 7,
	For_kind = 9,
	Expr_kind = 23,
};

struct _stmt {
	enum _stmt_kind kind;
	union {
		struct {
			expr_ty target;
			expr_ty iter;
			asdl_stmt_seq *body;
			asdl_stmt_seq *orelse;
		} For;

		struct {
			expr_ty target;
			operator_ty op;
			expr_ty value;
		} AugAssign;

		struct {
			identifier name;
			arguments_ty args;
			asdl_stmt_seq *body;
			asdl_expr_seq *decorator_list;
			expr_ty returns;
			string type_comment;
		} FunctionDef;

		struct {
			expr_ty value;
		} Return;

		struct {
			expr_ty value;
		} Expr;

		struct {
			asdl_expr_seq *targets;
			expr_ty value;
		} Assign;
	} v;
};

expr_ty
_PyAST_Name(identifier id, expr_context_ty ctx, PyArena *arena) {
	expr_ty p;
	assert(id);
	assert(ctx);
	p = (expr_ty) _PyArena_Malloc(arena, sizeof(*p));
	assert(p);
	p->kind = Name_kind;
	p->v.Name.id = id;
	p->v.Name.ctx = ctx;
	return p;
}

expr_ty
_PyAST_Call(expr_ty func, asdl_expr_seq *args, asdl_keyword_seq *keywords) {
	expr_ty p;
	assert(func);
	p = (expr_ty) _PyArena_Malloc(NULL, sizeof(*p));
	assert(p);
	p->kind = Call_kind;
	p->v.Call.func = func;
	p->v.Call.args = args;
	p->v.Call.keywords = keywords;
	return p;
}

stmt_ty
_PyAST_Expr(expr_ty value) {
	stmt_ty p;
	assert(value);
	p = (stmt_ty) _PyArena_Malloc(NULL, sizeof(*p));
	assert(p);
	p->kind = Expr_kind;
	p->v.Expr.value = value;
	return p;
}

// hack for now
#define asdl_type_ignore_seq void

mod_ty
_PyAST_Module(asdl_stmt_seq *body, asdl_type_ignore_seq *type_ignores,
		PyArena *arena) {
	mod_ty p;
	p = (mod_ty)_PyArena_Malloc(arena, sizeof(*p));
	assert(p);
	p->kind = Module_kind;
	p->v.Module.body = body;
	return p;
}

expr_ty
_PyAST_Constant(constant value) {
	expr_ty p;
	assert(value);
	p = (expr_ty) _PyArena_Malloc(NULL, sizeof(*p));
	assert(p);
	p->kind = Constant_kind;
	p->v.Constant.value = value;
	return p;
}

#endif
