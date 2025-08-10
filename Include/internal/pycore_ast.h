// TODO generate this file rather than manually create it
#pragma once

#include "pycore_ast_state.h"

#define _ASDL_SEQ_HEAD \
	Py_ssize_t size; \
	void **elements;

struct _keyword;
typedef struct _keyword *keyword_ty;


typedef struct {
	_ASDL_SEQ_HEAD
	keyword_ty typed_elements[1];
} asdl_keyword_seq;


typedef struct _expr *expr_ty;
typedef struct {
	_ASDL_SEQ_HEAD
	expr_ty typed_elements[1];
} asdl_expr_seq;

typedef struct _arg *arg_ty;

struct _withitem {
  expr_ty context_expr;
  expr_ty optional_vars;
};

typedef struct _withitem *withitem_ty;
typedef struct {
  _ASDL_SEQ_HEAD
  withitem_ty typed_elements[1];
} asdl_withitem_seq;


typedef struct {
	_ASDL_SEQ_HEAD
	arg_ty typed_elements[1];
} asdl_arg_seq;

#include "internal/pycore_asdl.h"

typedef struct _alias *alias_ty;

struct _alias {
  identifier name;
  identifier asname;
};

typedef struct {
  _ASDL_SEQ_HEAD
  alias_ty typed_elements[1];
} asdl_alias_seq;


typedef enum _unaryop { Invert=1, Not=2, UAdd=3, USub=4 } unaryop_ty;

typedef enum _operator {
	Add=1,
	Sub=2,
	Mult=3,
	Div=5,
	Mod=6,
	Pow=7,
	LShift=8,
	RShift=9,
	BitOr=10,
	BitXor=11,
	BitAnd=12,
	FloorDiv=13,
} operator_ty;

typedef enum _cmpop {
	Eq=1,
	Lt=3,
	Is=7,
	IsNot=8,
	In=9,
	NotIn=10,
} cmpop_ty;

typedef enum _expr_context { Load=1, Store=2, Del=3} expr_context_ty;

typedef struct _mod *mod_ty;

typedef struct _stmt *stmt_ty;

typedef struct _arguments *arguments_ty;

struct _arg {
	identifier arg;
};

struct _arguments {
  asdl_arg_seq *posonlyargs;
	asdl_arg_seq *args;
  arg_ty vararg;
  asdl_arg_seq *kwonlyargs;
  asdl_expr_seq *kw_defaults;
  arg_ty kwarg;
  asdl_expr_seq *defaults;
};

typedef struct {
	_ASDL_SEQ_HEAD
	stmt_ty typed_elements[1];
} asdl_stmt_seq;

typedef struct _excepthandler *excepthandler_ty;

enum _excepthandler_kind { ExceptHandler_kind=1};

struct _excepthandler {
  enum _excepthandler_kind kind;
  union {
    struct {
      expr_ty type;
      identifier name;
      asdl_stmt_seq *body;
    } ExceptHandler;
  } v;
};

typedef struct {
  _ASDL_SEQ_HEAD
  excepthandler_ty typed_elements[1];
} asdl_excepthandler_seq;


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
	BoolOp_kind=1,
	BinOp_kind=3,
	UnaryOp_kind=4,
	IfExp_kind=6,
	Dict_kind=7,
	Set_kind=8,
  Yield_kind=14,
	Compare_kind=16,
	Call_kind=17,
	Constant_kind=20,
	Attribute_kind=21,
	Subscript_kind=22,
	Starred_kind=23,
	Name_kind=24,
	List_kind=25,
	Tuple_kind=26,
	Slice_kind=27,
};

struct _expr {
	enum _expr_kind kind;
	union {
    struct {
      expr_ty test;
      expr_ty body;
      expr_ty orelse;
    } IfExp;

    struct {
      expr_ty value;
    } Yield;
		struct {
			expr_ty lower;
			expr_ty upper;
			expr_ty step;
		} Slice;

		struct {
			unaryop_ty op;
			expr_ty operand;
		} UnaryOp;

		struct {
			asdl_expr_seq *elts;
		} Set;

		struct {
			expr_ty value;
			expr_ty slice;
			expr_context_ty ctx;
		} Subscript;

		struct {
			asdl_expr_seq *keys;
			asdl_expr_seq *values;
		} Dict;

		struct {
			expr_ty value;
			expr_context_ty ctx;
		} Starred;

		struct {
			asdl_expr_seq *elts;
			expr_context_ty ctx;
		} List;

		struct {
			asdl_expr_seq *elts;
			expr_context_ty ctx;
		} Tuple;

		struct {
			expr_ty left;
			asdl_int_seq *ops;
			asdl_expr_seq *comparators;
		} Compare;

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
  AsyncFunctionDef_kind = 2,
	ClassDef_kind = 3,
	Return_kind = 4,
	Delete_kind = 5,
	Assign_kind = 6,
	AugAssign_kind = 7,
	For_kind = 9,
	While_kind = 11,
	If_kind = 12,
  With_kind = 13,
  Raise_kind = 16,
  Try_kind = 17,
  ImportFrom_kind = 20,
	Expr_kind = 23,
  Pass_kind = 24,
	Break_kind = 25,
};

struct _stmt {
	enum _stmt_kind kind;
	union {
    struct {
      expr_ty exc;
      expr_ty cause;
    } Raise;

    struct {
      asdl_stmt_seq *body;
      asdl_excepthandler_seq *handlers;
      asdl_stmt_seq *orelse;
      asdl_stmt_seq *finalbody;
    } Try;

    struct {
      asdl_withitem_seq *items;
      asdl_stmt_seq *body;
      string type_comment;
    } With;
    struct {
      identifier module;
      asdl_alias_seq *names;
      int level;
    } ImportFrom;

		struct {
			identifier name;
			asdl_expr_seq *bases;
			asdl_keyword_seq *keywords;
			asdl_stmt_seq *body;
			asdl_expr_seq *decorator_list;
		} ClassDef;

		struct {
			asdl_expr_seq *targets;
		} Delete;
		struct {
			expr_ty test;
			asdl_stmt_seq *body;
			asdl_stmt_seq *orelse;
		} While;
		struct {
			expr_ty test;
			asdl_stmt_seq *body;
			asdl_stmt_seq *orelse;
		} If;
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

struct _keyword {
	identifier arg;
	expr_ty value;
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

// defined in cpy/Python/Python-ast.c
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

stmt_ty _PyAST_While(expr_ty test, asdl_stmt_seq *body, asdl_stmt_seq *orelse) {
	stmt_ty p;
	if (!test) {
		assert(false);
	}
	p = (stmt_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->kind = While_kind;
	p->v.While.test = test;
	p->v.While.body = body;
	p->v.While.orelse = orelse;
	return p;
	assert(false);
}

expr_ty _PyAST_Yield(expr_ty value) {
  expr_ty p;
  p = (expr_ty) malloc(sizeof(*p));
  if (!p)
    return NULL;
  p->kind = Yield_kind;
  p->v.Yield.value = value;
  return p;
}

keyword_ty
_PyAST_keyword(identifier arg, expr_ty value) {
  keyword_ty p;
  if (!value) {
    fail(0);
  }
  p = (keyword_ty) malloc(sizeof(*p));
  if (!p)
    return NULL;
  p->arg = arg;
  p->value = value;
  return p;
}

stmt_ty
_PyAST_Raise(expr_ty exc, expr_ty cause) {
  stmt_ty p;
  p = (stmt_ty) malloc(sizeof(*p));
  if (!p)
    return NULL;
  p->kind = Raise_kind;
  p->v.Raise.exc = exc;
  p->v.Raise.cause = cause;
  return p;
}

static struct ast_state *get_ast_state(void);
int PyAST_Check(PyObject *obj);

expr_ty _PyAST_IfExp(expr_ty test, expr_ty body, expr_ty orelse) {
  expr_ty p;
  if (!test) {
    fail(0);
  }
  if (!body) {
    fail(0);
  }
  if (!orelse) {
    fail(0);
  }
  p = (expr_ty) malloc(sizeof(*p));
  if (!p)
    return NULL;
  p->kind = IfExp_kind;
  p->v.IfExp.test = test;
  p->v.IfExp.body = body;
  p->v.IfExp.orelse = orelse;
  return p;
}

