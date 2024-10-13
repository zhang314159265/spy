#pragma once

#include "code.h"
#include "opcode.h"
#include "pycapsule.h"
#include "Python/wordcode_helpers.h"

#define STACK_USE_GUIDELINE 30

// defined in cpy/Python/compile.c
PyObject *_Py_Mangle(PyObject *privateobj, PyObject *ident) {
  assert(privateobj == NULL); // TODO: support non-null privateobj
  Py_INCREF(ident);
  return ident;
}

#include "internal/pycore_symtable.h"

#define NEXT_BLOCK(C) { \
  if (compiler_next_block((C)) == NULL) \
    return 0; \
}

#define ADDOP_COMPARE(C, CMP) { \
  if (!compiler_addcompare((C), (cmpop_ty) (CMP))) \
    return 0; \
}

#define ADDOP_LOAD_CONST(C, O) { \
	if (!compiler_addop_load_const((C), (O))) \
		return 0; \
}

#define ADDOP_I(C, OP, O) { \
	if (!compiler_addop_i((C), (OP), (O))) \
		return 0; \
}

#define ADDOP(C, OP) { \
	if (!compiler_addop((C), (OP))) \
		return 0; \
}

#define ADDOP_JUMP(C, OP, O) { \
  if (!compiler_addop_j((C), (OP), (O))) \
    return 0; \
}

// Same as ADDOP_O but steals a reference
#define ADDOP_N(C, OP, O, TYPE) { \
  assert((OP) != LOAD_CONST); /* use ADDOP_LOAD_CONST_NEW */ \
  if (!compiler_addop_o((C), (OP), (C)->u->u_ ## TYPE, (O))) { \
    Py_DECREF((O)); \
    return 0; \
  } \
  Py_DECREF((O)); \
}

#define ADDOP_NAME(C, OP, O, TYPE) { \
  if (!compiler_addop_name((C), (OP), (C)->u->u_ ## TYPE, (O))) \
    return 0; \
}

enum {
	COMPILER_SCOPE_MODULE,
	COMPILER_SCOPE_CLASS,
	COMPILER_SCOPE_FUNCTION,
};

struct basicblock_;

struct instr {
	unsigned char i_opcode;
	int i_oparg;
	struct basicblock_ *i_target;
	int i_lineno;
};

typedef struct basicblock_ {
	// reverse order of allocation
	struct basicblock_ *b_list;

	int b_iused;
	int b_ialloc;
	struct instr *b_instr;

	struct basicblock_ *b_next;

	unsigned b_return : 1;
	unsigned b_nofallthrough : 1;
	unsigned b_exit : 1;
  unsigned b_visited : 1;

  // instruction offset for block, computed by assemble_jump_offsets()
  int b_offset;
} basicblock;

enum fblocktype {
  WHILE_LOOP,
  FOR_LOOP,
};

struct fblockinfo {
  enum fblocktype fb_type;
  basicblock *fb_block;
  basicblock *fb_exit;
  void *fb_datum;
};

struct compiler_unit {
	PySTEntryObject *u_ste;

	PyObject *u_name;
  PyObject *u_qualname;
	int u_scope_type;
	basicblock *u_blocks;
	basicblock *u_curblock;

	/* The following fields are dicts that map objects to
	   the index of them in co_XXX.  The index is used as
		 the argument for opcodes that refer to those collections.
	 */
	PyObject *u_consts; // all constants
	PyObject *u_names; // all names
	PyObject *u_varnames; // local variables

	int u_firstlineno;
	int u_lineno; // the lineno for the current stmt

  PyObject *u_private; // for private name mangling

  Py_ssize_t u_argcount;

  int u_nfblocks;
  struct fblockinfo u_fblock[CO_MAXBLOCKS];
};

struct compiler {
	struct symtable *c_st;
	struct compiler_unit *u; // compiler state for current block
	PyObject *c_const_cache;
  PyObject *c_stack;
};

struct assembler {
	PyObject *a_bytecode;
	PyObject *a_lnotab; 
	int a_offset;
	int a_nblocks;
	basicblock *a_entry;
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

static basicblock *compiler_new_block(struct compiler *c);
static int compiler_addop_i_line(struct compiler *c, int opcode, Py_ssize_t oparg, int lineno);
static int compiler_addop_i(struct compiler *c, int opcode, Py_ssize_t oparg);
static int compiler_jump_if(struct compiler *c, expr_ty e, basicblock *next, int cond);
static PyCodeObject *assemble(struct compiler *c, int addNone);


int
compiler_init(struct compiler *c) {
	memset(c, 0, sizeof(struct compiler));

	c->c_const_cache = PyDict_New();
	if (!c->c_const_cache) {
		return 0;
	}

  c->c_stack = PyList_New(0);
  if (!c->c_stack) {
    assert(false);
  }
	return 1;
}

// defined in cpy/Python/ast_opt.c
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
  case Attribute_kind:
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
  case Assign_kind:
  case FunctionDef_kind:
    // TODO: no-op for now
    break;
	case Expr_kind:
		CALL(astfold_expr, expr_ty, node_->v.Expr.value);
		break;
  case If_kind:
    assert(false);
  case For_kind:
    // TODO follow cpy
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

static Py_ssize_t compiler_add_o(PyObject *dict, PyObject *o);

static int
compiler_addop_name(struct compiler *c, int opcode, PyObject *dict,
    PyObject *o) {
  Py_ssize_t arg;

  PyObject *mangled = o; // TODO follow cpy
  arg = compiler_add_o(dict, mangled);
  Py_DECREF(mangled);
  if (arg < 0) 
    return 0;
  return compiler_addop_i(c, opcode, arg);
}

static int
compiler_addop_o(struct compiler *c, int opcode, PyObject *dict,
    PyObject *o) {
  Py_ssize_t arg = compiler_add_o(dict, o);
  if (arg < 0)
    return 0;
  return compiler_addop_i(c, opcode, arg);
}

static Py_ssize_t compiler_add_o(PyObject *dict, PyObject *o) {
	PyObject *v;
	Py_ssize_t arg;

	v = PyDict_GetItemWithError(dict, o);
	if (!v) {
		if (PyErr_Occurred()) {
			assert(false);
		}
		arg = PyDict_GET_SIZE(dict);
		v = PyLong_FromSsize_t(arg);
		if (!v) {
			return -1;
		}
		if (PyDict_SetItem(dict, o, v) < 0) {
			Py_DECREF(v);
			return -1;
		}
		Py_DECREF(v);
	}
	else
		arg = PyLong_AsLong(v);
	return arg;
}

static basicblock *
compiler_new_block(struct compiler *c)
{
	basicblock *b;
	struct compiler_unit *u;

	u = c->u;
	b = (basicblock *) PyObject_Calloc(1, sizeof(basicblock));
	if (b == NULL) {
		assert(false);
	}
	b->b_list = u->u_blocks;
	u->u_blocks = b;
	return b;
}

static PyObject *
list2dict(PyObject *list)
{
	Py_ssize_t i, n;
  PyObject *v, *k;
	PyObject *dict = PyDict_New();
	if (!dict) return NULL;

	n = PyList_Size(list);
	for (i = 0; i < n; i++) {
    v = PyLong_FromSsize_t(i);
    if (!v) {
      Py_DECREF(dict);
      return NULL;
    }
    k = PyList_GET_ITEM(list, i);
    if (PyDict_SetItem(dict, k, v) < 0) {
      assert(false);
    }
    Py_DECREF(v);
	}
	return dict;
}

static void
compiler_unit_free(struct compiler_unit *u) {

  basicblock *b, *next;

  b = u->u_blocks;
  while (b != NULL) {
    if (b->b_instr)
      PyObject_Free((void *) b->b_instr);
    next = b->b_list;
    PyObject_Free((void *) b);
    b = next;
  }

  Py_CLEAR(u->u_ste);
  Py_CLEAR(u->u_name);
  Py_CLEAR(u->u_consts);
  Py_CLEAR(u->u_names);
  Py_CLEAR(u->u_varnames);
  PyObject_Free(u);
}

static int
compiler_set_qualname(struct compiler *c) {
  // TODO: follow cpy
  PyObject *name;
  struct compiler_unit *u = c->u;
  // printf("name is %s\n", (char *) PyUnicode_DATA(u->u_name));

  Py_INCREF(u->u_name);
  name = u->u_name;

  u->u_qualname = name;
  return 1;
}

#define CAPSULE_NAME "compile.c compiler unit"

static int
compiler_enter_scope(struct compiler *c, identifier name,
		int scope_type, void *key, int lineno) {
	struct compiler_unit *u;
	basicblock *block;

	u = (struct compiler_unit *) PyObject_Calloc(1, sizeof(
		struct compiler_unit));
	if (!u) {
		assert(false);
	}
	u->u_scope_type = scope_type;

	// printf("sym table %p\n", PySymtable_Lookup(c->c_st, key));
	u->u_ste = PySymtable_Lookup(c->c_st, key);
	if (!u->u_ste) {
		assert(false);
	}
	Py_INCREF(name);
	u->u_name = name;
	u->u_varnames = list2dict(u->u_ste->ste_varnames);

  u->u_nfblocks = 0;
	u->u_consts = PyDict_New();
	if (!u->u_consts) {
		assert(false);
	}

	u->u_names = PyDict_New();
	if (!u->u_names) {
		assert(false);
	}

  u->u_private = NULL;

	if (c->u) {
    PyObject *capsule = PyCapsule_New(c->u, CAPSULE_NAME, NULL);
    if (!capsule || PyList_Append(c->c_stack, capsule) < 0) {
      assert(false);
    }
    Py_DECREF(capsule);
    u->u_private = c->u->u_private;
    Py_XINCREF(u->u_private);
	}
	c->u = u;

	block = compiler_new_block(c);
	if (block == NULL)
		return 0;
	c->u->u_curblock = block;

	if (u->u_scope_type != COMPILER_SCOPE_MODULE) {
    if (!compiler_set_qualname(c))
      return 0;
	}
	return 1;
}

static int
find_ann(asdl_stmt_seq *stmts) {
	return 0; // XXX hardcoded to 0 for now
}


static void compiler_exit_scope(struct compiler *c);
static int compiler_visit_stmt(struct compiler *c, stmt_ty s);

#undef VISIT

#define VISIT(C, TYPE, V) { \
	if (!compiler_visit_ ## TYPE((C), (V))) \
		return 0; \
}

#undef VISIT_SEQ 

#define VISIT_SEQ(C, TYPE, SEQ) { \
  int _i; \
  asdl_ ## TYPE ## _seq *seq = (SEQ); /* avoid variable capture */ \
  for (_i = 0; _i < asdl_seq_LEN(seq); _i++) { \
    TYPE ## _ty elt = (TYPE ## _ty) asdl_seq_GET(seq, _i); \
    if (!compiler_visit_ ## TYPE((C), elt)) \
      return 0; \
  } \
}

#define VISIT_IN_SCOPE(C, TYPE, V) { \
  if (!compiler_visit_ ## TYPE((C), (V))) { \
    compiler_exit_scope(c); \
    return 0; \
  } \
}

// Nop for now
#define SET_LOC(c, s)

static int compiler_visit_expr(struct compiler *c, expr_ty e);

// shared code between compiler_call and compiler_class
static int
compiler_call_helper(struct compiler *c,
		int n, /* Args already pushed */
		asdl_expr_seq *args,
		asdl_keyword_seq *keywords)
{
	Py_ssize_t i, nelts, nkwelts;

	nelts = asdl_seq_LEN(args);
	nkwelts = asdl_seq_LEN(keywords);
	// printf("nelts %ld, nkwelts %ld\n", nelts, nkwelts);

	// No * or ** args, so can use faster calling sequence
	for (i = 0; i < nelts; i++) {
		expr_ty elt = asdl_seq_GET(args, i);
		assert(elt->kind != Starred_kind);
		VISIT(c, expr, elt);
	}
	if (nkwelts) {
		assert(false);
	} else {
		ADDOP_I(c, CALL_FUNCTION, n + nelts);
		return 1;
	}
	assert(false);
}

// Return 1 if the method call was optimized, -1 if not and 0 on error
static int
maybe_optimize_method_call(struct compiler *c, expr_ty e) {
  Py_ssize_t argsl, i;
  expr_ty meth = e->v.Call.func;
  asdl_expr_seq *args = e->v.Call.args;

  if (meth->kind != Attribute_kind || meth->v.Attribute.ctx != Load ||
      asdl_seq_LEN(e->v.Call.keywords)) {
    return -1;
  }
  argsl = asdl_seq_LEN(args);
  if (argsl >= STACK_USE_GUIDELINE) {
    return -1;
  }
  for (i = 0; i < argsl; i++) {
    expr_ty elt = asdl_seq_GET(args, i);
    if (elt->kind == Starred_kind) {
      return -1;
    }
  }

  // optimize the code
  VISIT(c, expr, meth->v.Attribute.value);
  ADDOP_NAME(c, LOAD_METHOD, meth->v.Attribute.attr, names);
  VISIT_SEQ(c, expr, e->v.Call.args);
  ADDOP_I(c, CALL_METHOD, asdl_seq_LEN(e->v.Call.args));
  return 1;
}

static int
compiler_call(struct compiler *c, expr_ty e) {
  int ret = maybe_optimize_method_call(c, e);
  if (ret >= 0) {
    return ret;
  }
	VISIT(c, expr, e->v.Call.func);
	return compiler_call_helper(c, 0,
			e->v.Call.args,
			e->v.Call.keywords);
}

#define DEFAULT_BLOCK_SIZE 16
#define DEFAULT_LNOTAB_SIZE 16

// Returns the offset of the next instruction in the current block's
// b_instr array.  Resizes the b_instr as necessary.
// Returns -1 on failure
static int
compiler_next_instr(basicblock *b) {
	assert(b != NULL);
	if (b->b_instr == NULL) {
		b->b_instr = (struct instr *) PyObject_Calloc(
			DEFAULT_BLOCK_SIZE, sizeof(struct instr));
		if (b->b_instr == NULL) {
			assert(false);
		}
		b->b_ialloc = DEFAULT_BLOCK_SIZE;
	} else if (b->b_iused == b->b_ialloc) {
    struct instr *tmp;
    size_t oldsize, newsize;
    oldsize = b->b_ialloc * sizeof(struct instr);
    newsize = oldsize << 1;

    b->b_ialloc <<= 1;
    tmp = (struct instr *) PyObject_Realloc(
      (void *) b->b_instr, newsize);
    if (tmp == NULL) {
      assert(false);
    }
    b->b_instr = tmp;
    memset((char *) b->b_instr + oldsize, 0, newsize - oldsize);
	}
	return b->b_iused++;
}

// Add an opcode with no argument.
static int
compiler_addop_line(struct compiler* c, int opcode, int line)
{
	basicblock *b;
	struct instr *i;
	int off;
	assert(!HAS_ARG(opcode));
	off = compiler_next_instr(c->u->u_curblock);
	if (off < 0)
		return 0;
	b = c->u->u_curblock;
	i = &b->b_instr[off];
	i->i_opcode = opcode;
	i->i_oparg = 0;
	if (opcode == RETURN_VALUE)
		b->b_return = 1;
	i->i_lineno = line;
	return 1;
}

static int add_jump_to_block(basicblock *b, int opcode, int lineno, basicblock *target) {
  assert(HAS_ARG(opcode));
  assert(b != NULL);
  assert(target != NULL);

  int off = compiler_next_instr(b);
  struct instr *i = &b->b_instr[off];
  if (off < 0) {
    return 0;
  }
  i->i_opcode = opcode;
  i->i_target = target;
  i->i_lineno = lineno;
  return 1;
}

static int
compiler_addop_j(struct compiler *c, int opcode, basicblock *b) {
  return add_jump_to_block(c->u->u_curblock, opcode, c->u->u_lineno, b);
}

static int
compiler_addop(struct compiler *c, int opcode) {
	return compiler_addop_line(c, opcode, c->u->u_lineno);
}

// Add an opcode with an integer argument.
static int
compiler_addop_i_line(struct compiler *c, int opcode, Py_ssize_t oparg, int lineno) {
	struct instr *i;
	int off;
	assert(HAS_ARG(opcode));
	assert(0 <= oparg && oparg <= 2147483647);

	off = compiler_next_instr(c->u->u_curblock);
	if (off < 0)
		return 0;
	i = &c->u->u_curblock->b_instr[off];
	i->i_opcode = opcode;
	i->i_oparg = Py_SAFE_DOWNCAST(oparg, Py_ssize_t, int);
	i->i_lineno = lineno;
	return 1;
}

static int
compiler_addop_i(struct compiler *c, int opcode, Py_ssize_t oparg)
{
	return compiler_addop_i_line(c, opcode, oparg, c->u->u_lineno);
}

static int
compiler_nameop(struct compiler *c, identifier name, expr_context_ty ctx) {
	int op, scope;
	Py_ssize_t arg;
	PyObject *mangled;

	PyObject *dict = c->u->u_names;

	enum { OP_FAST, OP_GLOBAL, OP_DEREF, OP_NAME } optype;

	mangled = name; // TODO do the mangling

	op = 0;
	optype = OP_NAME;
	scope = _PyST_GetScope(c->u->u_ste, mangled);
	// printf("scope for %s is %d\n", PyUnicode_1BYTE_DATA(mangled), scope);
	switch (scope) {
  case FREE:
    assert(false);
  case CELL:
    assert(false);
  case LOCAL:
    if (c->u->u_ste->ste_type == FunctionBlock) {
      optype = OP_FAST;
    }
    break;
	case GLOBAL_IMPLICIT:
		if (c->u->u_ste->ste_type == FunctionBlock)
			optype = OP_GLOBAL;
		break;
  case GLOBAL_EXPLICIT:
    assert(false);
	default:
		// scope can be 0
		break;
	}

	switch (optype) {
  case OP_GLOBAL:
    switch (ctx) {
    case Load: op = LOAD_GLOBAL; break;
    case Store: op = STORE_GLOBAL; break;
    case Del: op = DELETE_GLOBAL; break;
    }
    break;
	case OP_NAME:
		switch (ctx) {
		case Load: op = LOAD_NAME; break;
		case Store: op = STORE_NAME; break;
		case Del: op = DELETE_NAME; break;
		}
		break;
  case OP_FAST:
    switch (ctx) {
    case Load: op = LOAD_FAST; break;
    case Store: op = STORE_FAST; break;
    case Del: op = DELETE_FAST; break;
    }
    ADDOP_N(c, op, mangled, varnames);
    return 1;
	default:
	  printf("can not handle '%s'\n", PyUnicode_1BYTE_DATA(mangled));
		assert(false);
	}

	assert(op);
	arg = compiler_add_o(dict, mangled);
	Py_DECREF(mangled);
	if (arg < 0)
		return 0;
	return compiler_addop_i(c, op, arg);
}

static PyObject *
merge_consts_recursive(struct compiler *c, PyObject *o) {
	PyObject *key = _PyCode_ConstantKey(o);
	if (key == NULL) {
		return NULL;
	}

	PyObject *t = PyDict_SetDefault(c->c_const_cache, key, key);
	if (t != key) {
		// o is registered in c_const_cache.  Just use it
		Py_XINCREF(t);
		Py_DECREF(key);
		return t;
	}

	if (PyTuple_CheckExact(o)) {
		assert(false);
	}
	return key;
}

static Py_ssize_t
compiler_add_const(struct compiler *c, PyObject *o) {
	PyObject *key = merge_consts_recursive(c, o);
	if (key == NULL) {
		return -1;
	}
  // printf("c is %p, o is %p\n", c, o);
  // printf("c->u is %p\n", c->u);
	Py_ssize_t arg = compiler_add_o(c->u->u_consts, key);
  // printf("after c is %p, o is %p\n", c, o);
	Py_DECREF(key);
	return arg;
}

static int
compiler_addop_load_const(struct compiler *c, PyObject *o) {
	Py_ssize_t arg = compiler_add_const(c, o);
	if (arg < 0)
		return 0;
	return compiler_addop_i(c, LOAD_CONST, arg);
}

static int
binop(operator_ty op) {
  switch (op) {
  case Add:
    return BINARY_ADD;
  case Sub:
    return BINARY_SUBTRACT;
  case Div:
    return BINARY_TRUE_DIVIDE;
  case Mod:
    return BINARY_MODULO;
  case Pow:
    return BINARY_POWER;
  case Mult:
    return BINARY_MULTIPLY;
  case FloorDiv:
    return BINARY_FLOOR_DIVIDE;
	case BitAnd:
		return BINARY_AND;
  case BitOr:
    return BINARY_OR;
  case BitXor:
    return BINARY_XOR;
  default:
    assert(false);
  }
}

static int compiler_addcompare(struct compiler *c, cmpop_ty op) {
  int cmp;
  switch (op) {
  case Eq:
    cmp = Py_EQ;
    break;
  case Lt:
    cmp = Py_LT;
    break;
  default:
    assert(false);
  }
  ADDOP_I(c, COMPARE_OP, cmp);
  return 1;
}

static int
compiler_compare(struct compiler *c, expr_ty e) {
  Py_ssize_t n;

  VISIT(c, expr, e->v.Compare.left);
  assert(asdl_seq_LEN(e->v.Compare.ops) > 0);
  n = asdl_seq_LEN(e->v.Compare.ops) - 1;

  if (n == 0) {
    VISIT(c, expr, (expr_ty) asdl_seq_GET(e->v.Compare.comparators, 0));
    ADDOP_COMPARE(c, asdl_seq_GET(e->v.Compare.ops, 0));
  } else {
    assert(false);
  }
  return 1;
}

static int
starunpack_helper(struct compiler *c, asdl_expr_seq *elts, int pushed,
    int build, int add, int extend, int tuple) {
  Py_ssize_t n = asdl_seq_LEN(elts);
  if (n > 2) {
    assert(false);
  }

  int big = n + pushed > STACK_USE_GUIDELINE;
  int seen_star = 0;
  for (Py_ssize_t i = 0; i < n; i++) {
    expr_ty elt = asdl_seq_GET(elts, i);
    if (elt->kind == Starred_kind) {
      seen_star = 1;
    }
  }
  if (!seen_star && !big) {
    for (Py_ssize_t i = 0; i < n; i++) {
      expr_ty elt = asdl_seq_GET(elts, i);
      VISIT(c, expr, elt);
    }
    if (tuple) {
      ADDOP_I(c, BUILD_TUPLE, n + pushed);
    } else {
      ADDOP_I(c, build, n + pushed);
    }
    return 1;
  }
  int sequence_built = 0;
  if (big) {
    assert(false);
  }
  for (Py_ssize_t i = 0; i < n; i++) {
    expr_ty elt = asdl_seq_GET(elts, i);
    if (elt->kind == Starred_kind) {
      if (sequence_built == 0) {
        ADDOP_I(c, build, i + pushed);
        sequence_built = 1;
      }
      VISIT(c, expr, elt->v.Starred.value);
      ADDOP_I(c, extend, 1);
    } else {
      VISIT(c, expr, elt);
      if (sequence_built) {
        ADDOP_I(c, add, 1);
      }
    }
  }
  assert(sequence_built);
  if (tuple) {
    ADDOP(c, LIST_TO_TUPLE);
  }
  return 1;
}

static int
compiler_tuple(struct compiler *c, expr_ty e) {
  asdl_expr_seq *elts = e->v.Tuple.elts;
  if (e->v.Tuple.ctx == Store) {
    assert(false);
  } else if (e->v.Tuple.ctx == Load) {
    return starunpack_helper(c, elts, 0, BUILD_LIST,
        LIST_APPEND, LIST_EXTEND, 1);
  } else {
    assert(false);
  }
  return 1;
}

static int
compiler_list(struct compiler *c, expr_ty e) {
  asdl_expr_seq *elts = e->v.List.elts;
  if (e->v.List.ctx == Store) {
    assert(false);
  } else if (e->v.List.ctx == Load) {
    return starunpack_helper(c, elts, 0, BUILD_LIST,
        LIST_APPEND, LIST_EXTEND, 0);
  } else {
    assert(false);
  }
  return 1;
}

static int
compiler_dict(struct compiler *c, expr_ty e) {
  Py_ssize_t i, n, elements;
  int have_dict;

  n = asdl_seq_LEN(e->v.Dict.values);
  have_dict = 0;
  elements = 0;
  for (i = 0; i < n; i++) {
    assert(false);
  }
  if (elements) {
    assert(false);
  }
  if (!have_dict) {
    ADDOP_I(c, BUILD_MAP, 0);
  }
  return 1;
}

static int
compiler_subscript(struct compiler *c, expr_ty e) {
  expr_context_ty ctx = e->v.Subscript.ctx;
  int op = 0;

  #if 0
  if (ctx == Load) {
    assert(false);
  }
  #endif

  switch (ctx) {
  case Load: op = BINARY_SUBSCR; break;
  case Store: op = STORE_SUBSCR; break;
  default:
    assert(false);
  }
  assert(op);
  VISIT(c, expr, e->v.Subscript.value);
  VISIT(c, expr, e->v.Subscript.slice);
  ADDOP(c, op);
  return 1;
}

static int
compiler_set(struct compiler *c, expr_ty e) {
  return starunpack_helper(c, e->v.Set.elts, 0, BUILD_SET,
    SET_ADD, SET_UPDATE, 0);
}

static int
compiler_visit_expr1(struct compiler *c, expr_ty e) {
	switch (e->kind) {
	case Call_kind:
		return compiler_call(c, e);
	case Name_kind:
		return compiler_nameop(c, e->v.Name.id, e->v.Name.ctx);
	case Constant_kind:
		// printf("const %s\n", PyUnicode_1BYTE_DATA(e->v.Constant.value));
		ADDOP_LOAD_CONST(c, e->v.Constant.value);
		break;
  case BinOp_kind:
    VISIT(c, expr, e->v.BinOp.left);
    VISIT(c, expr, e->v.BinOp.right);
    ADDOP(c, binop(e->v.BinOp.op));
    break;
  case Attribute_kind:
    VISIT(c, expr, e->v.Attribute.value);
    switch (e->v.Attribute.ctx) {
    case Load: 
    {
      ADDOP_NAME(c, LOAD_ATTR, e->v.Attribute.attr, names);
      break;
    }
    default:
      assert(false);
    }
    break;
  case Compare_kind:
    return compiler_compare(c, e);
  case Tuple_kind:
    return compiler_tuple(c, e);
  case List_kind:
    return compiler_list(c, e);
  case Dict_kind:
    return compiler_dict(c, e);
  case Subscript_kind:
    return compiler_subscript(c, e);
  case Set_kind:
    return compiler_set(c, e);
	default:
		assert(false);
	}
	return 1;
}

static int
compiler_visit_expr(struct compiler *c, expr_ty e)
{
	int res = compiler_visit_expr1(c, e);
	return res;
}

static int
compiler_visit_stmt_expr(struct compiler *c, expr_ty value)
{
	if (value->kind == Constant_kind) {
		assert(false);
	}

	VISIT(c, expr, value);
	c->u->u_lineno = -1;
	ADDOP(c, POP_TOP);
	return 1;
}

static int
compiler_decorators(struct compiler *c, asdl_expr_seq *decos) {
  if (!decos)
    return 1;
  assert(false);
}

static int
compiler_make_closure(struct compiler *c, PyCodeObject *co, Py_ssize_t flags,
    PyObject *qualname) {
  Py_ssize_t i, free = PyCode_GetNumFree(co);
  if (qualname == NULL)
    qualname = co->co_name;

  if (free) {
    assert(false);
  }
  ADDOP_LOAD_CONST(c, (PyObject*) co);
  ADDOP_LOAD_CONST(c, qualname);
  ADDOP_I(c, MAKE_FUNCTION, flags);
  return 1;
}

static Py_ssize_t
compiler_default_arguments(struct compiler *c, arguments_ty args) {
  Py_ssize_t funcflags = 0;
  #if 0
  if (args->defaults && asdl_seq_LEN(args->defaults) > 0) {
    assert(false);
  }
  #endif
  #if 0
  if (args->kwonlyargs) {
    assert(false);
  }
  #endif
  return funcflags;
}

static int
compiler_function(struct compiler *c, stmt_ty s, int is_async) {
  PyCodeObject *co;
  PyObject *qualname, *docstring = NULL;
  arguments_ty args;
  expr_ty returns;
  identifier name;
  asdl_expr_seq *decos;
  asdl_stmt_seq *body;
  Py_ssize_t i, funcflags;
  int scope_type;
  int firstlineno;

  if (is_async) {
    assert(false);
  } else {
    assert(s->kind == FunctionDef_kind);

    args = s->v.FunctionDef.args;
    returns = s->v.FunctionDef.returns;
    decos = s->v.FunctionDef.decorator_list;
    name = s->v.FunctionDef.name;
    body = s->v.FunctionDef.body;

    scope_type = COMPILER_SCOPE_FUNCTION;
  }

  #if 0
  if (!compiler_check_debug_args(c, args))
    return 0;
  #endif

  if (!compiler_decorators(c, decos))
    return 0;

  firstlineno = -1;

  funcflags = compiler_default_arguments(c, args);
  if (funcflags == -1) {
    return 0;
  }

  if (!compiler_enter_scope(c, name, scope_type, (void *) s, firstlineno)) {
    return 0;
  }
  if (compiler_add_const(c, Py_None) < 0) {
    assert(false);
  }

  c->u->u_argcount = asdl_seq_LEN(args->args);
  for (i = docstring ? 1 : 0; i < asdl_seq_LEN(body); i++) {
    VISIT_IN_SCOPE(c, stmt, (stmt_ty) asdl_seq_GET(body, i));
  }
  co = assemble(c, 1);
  qualname = c->u->u_qualname;
  Py_INCREF(qualname);
  compiler_exit_scope(c);
  if (co == NULL) {
    assert(false);
  }
  
  if (!compiler_make_closure(c, co, funcflags, qualname)) {
    assert(false);
  }
  Py_DECREF(qualname);
  Py_DECREF(co);

  // decorators
  for (i = 0; i < asdl_seq_LEN(decos); i++) {
    assert(false);
  }

  return compiler_nameop(c, name, Store);
}

static int
compiler_unwind_fblock_stack(struct compiler *c, int preserve_tos, struct fblockinfo **loop) {
  if (c->u->u_nfblocks == 0) {
    return 1;
  }
  struct fblockinfo *top = &c->u->u_fblock[c->u->u_nfblocks - 1];
  if (loop != NULL && (top->fb_type == WHILE_LOOP || top->fb_type == FOR_LOOP)) {
    *loop = top;
    return 1;
  }
  assert(false);
}

static basicblock *
compiler_next_block(struct compiler *c) {
  basicblock *block = compiler_new_block(c);
  if (block == NULL)
    return NULL;
  c->u->u_curblock->b_next = block;
  c->u->u_curblock = block;
  return block;
}

static int
compiler_return(struct compiler *c, stmt_ty s) {
  int preserve_tos = ((s->v.Return.value != NULL) &&
      (s->v.Return.value->kind != Constant_kind));
  // printf("preserve_tos is %d\n", preserve_tos);
  if (c->u->u_ste->ste_type != FunctionBlock) {
    assert(false);
  }
  if (preserve_tos) {
    VISIT(c, expr, s->v.Return.value);
  } else {
    assert(false);
  }

  if (!compiler_unwind_fblock_stack(c, preserve_tos, NULL))
    return 0;

  if (s->v.Return.value == NULL) {
    assert(false);
  }
  else if (!preserve_tos) {
    assert(false);
  }
  ADDOP(c, RETURN_VALUE);
  NEXT_BLOCK(c);

  return 1;
}

static int
inplace_binop(operator_ty op) {
  switch (op) {
  case Add:
    return INPLACE_ADD;
  default:
    assert(false);
  }
}

static int
compiler_augassign(struct compiler *c, stmt_ty s) {
  assert(s->kind == AugAssign_kind);
  expr_ty e = s->v.AugAssign.target;

  switch (e->kind) {
  case Name_kind:
    if (!compiler_nameop(c, e->v.Name.id, Load))
      return 0;
    break;
  default:
    assert(false);
  }

  VISIT(c, expr, s->v.AugAssign.value);
  ADDOP(c, inplace_binop(s->v.AugAssign.op));
  switch (e->kind) {
  case Name_kind:
    return compiler_nameop(c, e->v.Name.id, Store);
  default:
    assert(false);
  }
  return 1;
}

static int
compiler_push_fblock(struct compiler *c, enum fblocktype t, basicblock *b,
    basicblock *exit, void *datum) {
  struct fblockinfo *f;
  if (c->u->u_nfblocks >= CO_MAXBLOCKS) {
    assert(false);
  }
  f = &c->u->u_fblock[c->u->u_nfblocks++];
  f->fb_type = t;
  f->fb_block = b;
  f->fb_exit = exit;
  f->fb_datum = datum;
  return 1;
}

static basicblock *
compiler_use_next_block(struct compiler *c, basicblock *block) {
  assert(block != NULL);
  c->u->u_curblock->b_next = block;
  c->u->u_curblock = block;
  return block;
}

static void
compiler_pop_fblock(struct compiler *c, enum fblocktype t, basicblock *b) {
  struct compiler_unit *u = c->u;
  assert(u->u_nfblocks > 0);
  u->u_nfblocks--;
  assert(u->u_fblock[u->u_nfblocks].fb_type == t);
  assert(u->u_fblock[u->u_nfblocks].fb_block == b);
}

static int
compiler_for(struct compiler *c, stmt_ty s) {
  basicblock *start, *body, *cleanup, *end;

  start = compiler_new_block(c);
  body = compiler_new_block(c);
  cleanup = compiler_new_block(c);
  end = compiler_new_block(c);
  if (start == NULL || body == NULL || cleanup == NULL || end == NULL) {
    return 0;
  }
  if (!compiler_push_fblock(c, FOR_LOOP, start, end, NULL)) {
    return 0;
  }
  VISIT(c, expr, s->v.For.iter);
  ADDOP(c, GET_ITER);
  compiler_use_next_block(c, start);
  ADDOP_JUMP(c, FOR_ITER, cleanup);
  compiler_use_next_block(c, body);

  VISIT(c, expr, s->v.For.target);
  VISIT_SEQ(c, stmt, s->v.For.body);
  ADDOP_JUMP(c, JUMP_ABSOLUTE, start);
  compiler_use_next_block(c, cleanup);

  compiler_pop_fblock(c, FOR_LOOP, start);
  
  VISIT_SEQ(c, stmt, s->v.For.orelse);
  compiler_use_next_block(c, end);
  return 1;
}

static int
compiler_jump_if(struct compiler *c, expr_ty e, basicblock *next, int cond) {
  switch (e->kind) {
  case UnaryOp_kind:
    assert(false);
  case BoolOp_kind:
    assert(false);
  case IfExp_kind:
    assert(false);
  case Compare_kind: {
    Py_ssize_t n = asdl_seq_LEN(e->v.Compare.ops) - 1;
    if (n > 0) {
      assert(false);
    }
    break;
  }
  default:
    break;
  }

  VISIT(c, expr, e);
  ADDOP_JUMP(c, cond ? POP_JUMP_IF_TRUE : POP_JUMP_IF_FALSE, next);
  NEXT_BLOCK(c);
  return 1;
}

static int
compiler_if(struct compiler *c, stmt_ty s) {
  basicblock *end, *next;
  assert(s->kind == If_kind);
  end = compiler_new_block(c);
  if (end == NULL) {
    return 0;
  }
  if (asdl_seq_LEN(s->v.If.orelse)) {
    assert(false);
  } else {
    next = end;
  }
  if (!compiler_jump_if(c, s->v.If.test, next, 0)) {
    return 0;
  }
  VISIT_SEQ(c, stmt, s->v.If.body);
  if (asdl_seq_LEN(s->v.If.orelse)) {
    assert(false);
  }
  compiler_use_next_block(c, end);
  return 1;
}

static int
compiler_while(struct compiler *c, stmt_ty s) {
  basicblock *loop, *body, *end, *anchor = NULL;
  loop = compiler_new_block(c);
  body = compiler_new_block(c);
  anchor = compiler_new_block(c);
  end = compiler_new_block(c);
  if (loop == NULL || body == NULL || anchor == NULL || end == NULL) {
    return 0;
  }
  compiler_use_next_block(c, loop);
  if (!compiler_push_fblock(c, WHILE_LOOP, loop, end, NULL)) {
    return 0;
  }
  if (!compiler_jump_if(c, s->v.While.test, anchor, 0)) {
    return 0;
  }

  compiler_use_next_block(c, body);
  VISIT_SEQ(c, stmt, s->v.While.body);
  if (!compiler_jump_if(c, s->v.While.test, body, 1)) {
    return 0;
  }

  compiler_pop_fblock(c, WHILE_LOOP, loop);

  compiler_use_next_block(c, anchor);
  if (s->v.While.orelse) {
    assert(false);
  }
  compiler_use_next_block(c, end);

  return 1;
}

static int compiler_unwind_fblock(struct compiler *c, struct fblockinfo *info, int preserve_tos) {
  switch (info->fb_type) {
  case WHILE_LOOP:
    return 1;
  default:
    assert(false);
  }
  assert(false);
}

static int
compiler_break(struct compiler *c) {
  struct fblockinfo *loop = NULL;
  if (!compiler_unwind_fblock_stack(c, 0, &loop)) {
    return 0;
  }
  if (loop == NULL) {
    assert(false && "break outside loop");
  }
  if (!compiler_unwind_fblock(c, loop, 0)) {
    return 0;
  }
  ADDOP_JUMP(c, JUMP_ABSOLUTE, loop->fb_exit);
  NEXT_BLOCK(c);
  return 1;
}

static int
compiler_visit_stmt(struct compiler *c, stmt_ty s) {
	Py_ssize_t i, n;

	SET_LOC(c, s);

	switch (s->kind) {
  case FunctionDef_kind:
    return compiler_function(c, s, 0);
	case Expr_kind:
		return compiler_visit_stmt_expr(c, s->v.Expr.value);
  case Assign_kind:
    n = asdl_seq_LEN(s->v.Assign.targets);
    VISIT(c, expr, s->v.Assign.value);
    for (i = 0; i < n; i++) {
      if (i < n - 1)
        ADDOP(c, DUP_TOP);

      VISIT(c, expr,
          (expr_ty) asdl_seq_GET(s->v.Assign.targets, i));
    }
    break;
  case AugAssign_kind:
    return compiler_augassign(c, s);
  case Return_kind:
    return compiler_return(c, s);
  case For_kind:
    return compiler_for(c, s);
  case If_kind:
    return compiler_if(c, s);
  case While_kind:
    return compiler_while(c, s);
  case Break_kind:
    return compiler_break(c);
	default:
		assert(false);
	}
	return 1;
}

// Compile a sequence of statements, checking for a docstring
// and for annotations
static int
compiler_body(struct compiler *c, asdl_stmt_seq *stmts) {
	int i = 0;

	if (!asdl_seq_LEN(stmts))
		return 1;
	for (; i < asdl_seq_LEN(stmts); i++)
		VISIT(c, stmt, (stmt_ty) asdl_seq_GET(stmts, i));
	return 1;
}

static int
normalize_basic_block(basicblock *bb) {
	for (int i = 0; i < bb->b_iused; i++) {
		switch (bb->b_instr[i].i_opcode) {
		case RETURN_VALUE:
		case RAISE_VARARGS:
		case RERAISE:
			bb->b_exit = 1;
			bb->b_nofallthrough = 1;
			break;
		case JUMP_ABSOLUTE:
		case JUMP_FORWARD:
      bb->b_nofallthrough = 1;
      /* fall through */
		case POP_JUMP_IF_FALSE:
		case POP_JUMP_IF_TRUE:
		case JUMP_IF_FALSE_OR_POP:
		case JUMP_IF_TRUE_OR_POP:
		case FOR_ITER:
      if (i != bb->b_iused - 1) {
        assert(false);
      }
      /* Skip over empty basic blocks */
      while (bb->b_instr[i].i_target->b_iused == 0) {
        bb->b_instr[i].i_target = bb->b_instr[i].i_target->b_next;
      }
		}
	}
	return 0;
}

static int
extend_block(basicblock *bb) {
  return 0; // TODO no op for now since this is just an optimization
	if (bb->b_iused == 0) {
		return 0;
	}
	struct instr *last = &bb->b_instr[bb->b_iused - 1];
	if (last->i_opcode != JUMP_ABSOLUTE && last->i_opcode != JUMP_FORWARD) {
		return 0;
	}
	assert(false);
}

static int
insert_generator_prefix(struct compiler *c, basicblock *entryblock) {
	// not implemented yet
	return 0;
}

#define DEFAULT_CODE_SIZE 128

static int
assemble_init(struct assembler *a, int nblocks, int firstlineno) {
	memset(a, 0, sizeof(struct assembler));
	a->a_bytecode = PyBytes_FromStringAndSize(NULL, DEFAULT_CODE_SIZE);
	if (a->a_bytecode == NULL) {
		assert(false);
	}

	a->a_lnotab = PyBytes_FromStringAndSize(NULL, DEFAULT_LNOTAB_SIZE);
	if (a->a_lnotab == NULL) {
		assert(false);
	}
	return 1;
}

static PyObject *
consts_dict_keys_inorder(PyObject *dict) {
	PyObject *consts, *k, *v;
	Py_ssize_t i, pos = 0, size = PyDict_GET_SIZE(dict);

	consts = PyList_New(size);
	if (consts == NULL)
		return NULL;
	while (PyDict_Next(dict, &pos, &k, &v)) {
		i = PyLong_AS_LONG(v);
		if (PyTuple_CheckExact(k)) {
			k = PyTuple_GET_ITEM(k, 1);
		}
		Py_INCREF(k);
		assert(i < size);
		assert(i >= 0);
		PyList_SET_ITEM(consts, i, k);
	}
	return consts;
}

static int
optimize_basic_block(struct compiler *c, basicblock *bb, PyObject *consts) {
	// assert(false);
	return 0;
}

static int
optimize_cfg(struct compiler *c, struct assembler *a, PyObject *consts) {
	for (basicblock *b = a->a_entry; b != NULL; b = b->b_next) {
		if (optimize_basic_block(c, b, consts)) {
			return -1;
		}
		// assert(false);
	}
	// assert(false);
	return 0;
}

static int
assemble_emit(struct assembler *a, struct instr *i) {
	int size, arg = 0;
	Py_ssize_t len = PyBytes_GET_SIZE(a->a_bytecode);
	_Py_CODEUNIT *code;
	arg = i->i_oparg;
	size = instrsize(arg);

	if (a->a_offset + size >= len / (int) sizeof(_Py_CODEUNIT)) {
		assert(false);
	}
	code = (_Py_CODEUNIT *) PyBytes_AS_STRING(a->a_bytecode) + a->a_offset;
	a->a_offset += size;
	write_op_arg(code, i->i_opcode, arg, size);
	return 1;
}

static int
merge_const_one(struct compiler *c, PyObject **obj) {
	PyObject *key = _PyCode_ConstantKey(*obj);
	if (key == NULL) {
		return 0;
	}

	PyObject *t = PyDict_SetDefault(c->c_const_cache, key, key);
	Py_DECREF(key);
	if (t == NULL) {
		return 0;
	}
	if (t == key) {  // obj is new constant.
		return 1;
	}

	if (PyTuple_CheckExact(t)) {
		t = PyTuple_GET_ITEM(t, 1);
	}

	Py_INCREF(t);
	Py_DECREF(*obj);
	*obj = t;
	return 1;
}

static PyObject *
dict_keys_inorder(PyObject *dict, Py_ssize_t offset)
{
	PyObject *tuple, *k, *v;
	Py_ssize_t i, pos = 0, size = PyDict_GET_SIZE(dict);

	tuple = PyTuple_New(size);
	if (tuple == NULL)
		return NULL;
	while (PyDict_Next(dict, &pos, &k, &v)) {
		i = PyLong_AS_LONG(v);
		Py_INCREF(k);
		assert((i - offset) < size);
		assert((i - offset) >= 0);
		PyTuple_SET_ITEM(tuple, i - offset, k);
	}
	return tuple;
}

static int
compute_code_flags(struct compiler *c) {
  // TODO follow cpy
  PySTEntryObject *ste = c->u->u_ste;
  int flags = 0;

  if (ste->ste_type == FunctionBlock) {
    flags |= CO_NEWLOCALS | CO_OPTIMIZED;
  }
  return flags;
}

/* Find the flow path that needs the largest stack.  We assume that
 * cycles in the flow graph have no net effect on the stack depth.
 */
static int
stackdepth(struct compiler *c) {
  // TODO: do a real implementation
  return 128;
}

static PyCodeObject *
makecode(struct compiler *c, struct assembler *a, PyObject *consts)
{
  PyCodeObject *co = NULL;
	PyObject *names = NULL;
	PyObject *varnames = NULL;
  PyObject *freevars = NULL;
	// PyObject *cellvars = NULL;
  Py_ssize_t nlocals;
  int nlocals_int;
  int flags;
  int maxdepth;

	names = dict_keys_inorder(c->u->u_names, 0);
	varnames = dict_keys_inorder(c->u->u_varnames, 0);
	if (!names || !varnames) {
		assert(false);
	}
	// cellvars = dict_keys_inorder(c->u->u_cellvars, 0);

  freevars = PyTuple_New(0); // TODO follow cpy
  if (!freevars) {
    assert(false);
  }

	if (!merge_const_one(c, &names) ||
			!merge_const_one(c, &varnames)) {
	  assert(false);
	}

  nlocals = PyDict_GET_SIZE(c->u->u_varnames);
  assert(nlocals < INT_MAX);
  nlocals_int = Py_SAFE_DOWNCAST(nlocals, Py_ssize_t, int);

  flags = compute_code_flags(c);
  if (flags < 0)
    assert(false);

  consts = PyList_AsTuple(consts); /* PyCode_New requires a tuple */
  if (consts == NULL) {
    assert(false);
  }

  if (!merge_const_one(c, &consts)) {
    assert(false);
  }

  maxdepth = stackdepth(c);
  if (maxdepth < 0) {
    assert(false);
  }

  co = PyCode_NewWithPosOnlyArgs(
    nlocals_int, maxdepth, flags, a->a_bytecode, consts,
    names, varnames, freevars, c->u->u_name,
    c->u->u_firstlineno, a->a_lnotab);
  Py_DECREF(consts);
  Py_XDECREF(names);
  Py_XDECREF(varnames);
  // Py_XDECREF(name);
  // Py_XDECREF(freevars);
  // Py_XDECREF(cellvars);

  return co;
}

static void
assemble_free(struct assembler *a) {
	Py_XDECREF(a->a_bytecode);
	Py_XDECREF(a->a_lnotab);
}

static void
normalize_jumps(struct assembler *a) {
  for (basicblock *b = a->a_entry; b != NULL; b = b->b_next) {
    b->b_visited = 0;
  }
  for (basicblock *b = a->a_entry; b != NULL; b = b->b_next) {
    b->b_visited = 1;
    if (b->b_iused == 0) {
      continue;
    }
    struct instr *last = &b->b_instr[b->b_iused - 1];
    if (last->i_opcode == JUMP_ABSOLUTE) {
      if (last->i_target->b_visited == 0) {
        last->i_opcode = JUMP_FORWARD;
      }
    }
    if (last->i_opcode == JUMP_FORWARD) {
      if (last->i_target->b_visited == 1) {
        last->i_opcode = JUMP_ABSOLUTE;
      }
    }
  }
}

static int
blocksize(basicblock *b) {
  int i;
  int size = 0;

  for (i = 0; i < b->b_iused; i++) {
    size += instrsize(b->b_instr[i].i_oparg);
  }
  return size;
}

static inline int
is_relative_jump(struct instr *i) {
  // hasjrel. Check cpy/Lib/opcode.py
  // TODO follow cpy
  int c = i->i_opcode;
  return c == FOR_ITER || c == JUMP_FORWARD
    || c == SETUP_FINALLY || c == SETUP_WITH
    || c == SETUP_ASYNC_WITH;
}

static inline int
is_absolute_jump(struct instr *i) {
  // no such function in cpy
  int c = i->i_opcode;

  return c == JUMP_IF_FALSE_OR_POP
    || c == JUMP_IF_TRUE_OR_POP || c == JUMP_ABSOLUTE
    || c == POP_JUMP_IF_FALSE || c == POP_JUMP_IF_TRUE
    || c == JUMP_IF_NOT_EXC_MATCH;
}

static inline int
is_jump(struct instr *i) {
  // hasjrel || hasjabs. Check cpy/Lib/opcode.py
  // TODO follow cpy
  return is_relative_jump(i) || is_absolute_jump(i);
}

static void
assemble_jump_offsets(struct assembler *a, struct compiler *c) {
  basicblock *b;
  int bsize, totsize, extended_arg_recompile;
  int i;

  do {
    totsize = 0;
    for (basicblock *b = a->a_entry; b != NULL; b = b->b_next) {
      bsize = blocksize(b);
      b->b_offset = totsize;
      totsize += bsize;
    }
    extended_arg_recompile = 0;
    for (b = c->u->u_blocks; b != NULL; b = b->b_list) {
      bsize = b->b_offset;
      for (i = 0; i < b->b_iused; i++) {
        struct instr *instr = &b->b_instr[i];
        int isize = instrsize(instr->i_oparg);
        bsize += isize;
        if (is_jump(instr)) {
          instr->i_oparg = instr->i_target->b_offset;
          if (is_relative_jump(instr)) {
            instr->i_oparg -= bsize;
          }
          if (instrsize(instr->i_oparg) != isize) {
            extended_arg_recompile = 1;
          }
        }
      }
    }
  } while (extended_arg_recompile);
}

static PyCodeObject *
assemble(struct compiler *c, int addNone) {
	basicblock *b, *entryblock;
	struct assembler a;	
	int j, nblocks;
	PyCodeObject *co = NULL;
	PyObject *consts = NULL;
	memset(&a, 0, sizeof(struct assembler));

	if (!c->u->u_curblock->b_return) {
		c->u->u_lineno = -1;
		if (addNone)
			ADDOP_LOAD_CONST(c, Py_None);
		ADDOP(c, RETURN_VALUE);
	}

	for (basicblock *b = c->u->u_blocks; b != NULL; b = b->b_list) {
		if (normalize_basic_block(b)) {
			return NULL;
		}
	}
	
	for (basicblock *b = c->u->u_blocks; b != NULL; b = b->b_list) {
		if (extend_block(b)) {
			return NULL;
		}
	}

	nblocks = 0;
	entryblock = NULL;
	for (b = c->u->u_blocks; b != NULL; b = b->b_list) {
		nblocks++;
		entryblock = b;
	}
	assert(entryblock != NULL);

	if (insert_generator_prefix(c, entryblock)) {
		assert(false);
	}
	
	if (!assemble_init(&a, nblocks, c->u->u_firstlineno))
		assert(false);

	a.a_entry = entryblock;
	a.a_nblocks = nblocks;

	consts = consts_dict_keys_inorder(c->u->u_consts);
	if (consts == NULL) {
		assert(false);
	}

	if (optimize_cfg(c, &a, consts)) {
		assert(false);
	}

  normalize_jumps(&a);
  assemble_jump_offsets(&a, c);

	// Emit code
	for (b = entryblock; b != NULL; b = b->b_next) {
		for (j = 0; j < b->b_iused; j++) {
			if (!assemble_emit(&a, &b->b_instr[j])) {
				assert(false);
			}
		}
	}

	if (_PyBytes_Resize(&a.a_bytecode, a.a_offset * sizeof(_Py_CODEUNIT)) < 0) {
		assert(false);
	}
	if (!merge_const_one(c, &a.a_bytecode)) {
		assert(false);
	}

	co = makecode(c, &a, consts);
 error:
  Py_XDECREF(consts);
	assemble_free(&a);
	return co;
}

static void
compiler_exit_scope(struct compiler *c)
{
  compiler_unit_free(c->u);
  // Restore c->u to the parent unit
  Py_ssize_t n = PyList_GET_SIZE(c->c_stack) - 1;
  if (n >= 0) {
    PyObject *capsule = PyList_GET_ITEM(c->c_stack, n);
    c->u = (struct compiler_unit *) PyCapsule_GetPointer(capsule, CAPSULE_NAME);
    assert(c->u);
    if (PySequence_DelItem(c->c_stack, n) < 0) {
      assert(false);
    }
  } else {
    c->u = NULL;
  }
}

static PyCodeObject *
compiler_mod(struct compiler *c, mod_ty mod) {
	PyCodeObject *co;
	int addNone = 1;
	static PyObject *module;

	if (!module) {
		module = PyUnicode_InternFromString("<module>");
		if (!module)
			return NULL;
	}

	if (!compiler_enter_scope(c, module, COMPILER_SCOPE_MODULE, mod, 1))
		return NULL;

	switch (mod->kind) {
	case Module_kind:
		if (!compiler_body(c, mod->v.Module.body)) {
			assert(false);
		}
		break;
	default:
		assert(false);
	}
	co = assemble(c, addNone);
  compiler_exit_scope(c);
  return co;
}

static void
compiler_free(struct compiler *c) {
  if (c->c_st)
    _PySymtable_Free(c->c_st);

  Py_DECREF(c->c_const_cache);
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

	co = compiler_mod(&c, mod);
 finally:
  compiler_free(&c);
  assert(co);
  return co;
}

