#pragma once

#include "code.h"
#include "opcode.h"
#include "Python/wordcode_helpers.h"

// defined in cpy/Python/compile.c
PyObject *_Py_Mangle(PyObject *privateobj, PyObject *ident) {
  assert(privateobj == NULL); // TODO: support non-null privateobj
  Py_INCREF(ident);
  return ident;
}

#include "internal/pycore_symtable.h"

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
} basicblock;

struct compiler_unit {
	PySTEntryObject *u_ste;

	PyObject *u_name;
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
};

struct compiler {
	struct symtable *c_st;
	struct compiler_unit *u; // compiler state for current block
	PyObject *c_const_cache;
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

static int compiler_addop_i_line(struct compiler *c, int opcode, Py_ssize_t oparg, int lineno);
static int compiler_addop_i(struct compiler *c, int opcode, Py_ssize_t oparg);



int
compiler_init(struct compiler *c) {
	memset(c, 0, sizeof(struct compiler));

	c->c_const_cache = PyDict_New();
	if (!c->c_const_cache) {
		return 0;
	}
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

static Py_ssize_t
compiler_add_o(PyObject *dict, PyObject *o) {
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
	PyObject *dict = PyDict_New();
	if (!dict) return NULL;

	n = PyList_Size(list);
	for (i = 0; i < n; i++) {
		assert(false);
	}
	return dict;
}

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

	u->u_consts = PyDict_New();
	if (!u->u_consts) {
		assert(false);
	}

	u->u_names = PyDict_New();
	if (!u->u_names) {
		assert(false);
	}

	if (c->u) {
		assert(false);
	}
	c->u = u;

	block = compiler_new_block(c);
	if (block == NULL)
		return 0;
	c->u->u_curblock = block;

	if (u->u_scope_type != COMPILER_SCOPE_MODULE) {
		assert(false);
	}
	return 1;
}

static int
find_ann(asdl_stmt_seq *stmts) {
	return 0; // XXX hardcoded to 0 for now
}

#undef VISIT

#define VISIT(C, TYPE, V) { \
	if (!compiler_visit_ ## TYPE((C), (V))) \
		return 0; \
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

static int
compiler_call(struct compiler *c, expr_ty e) {
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
		assert(false);
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
	case GLOBAL_IMPLICIT:
		if (c->u->u_ste->ste_type == FunctionBlock)
			optype = OP_GLOBAL;
		break;
		assert(false);	
	default:
		assert(false);
		// scope can be 0
		break;
	}

	switch (optype) {
	case OP_NAME:
		switch (ctx) {
		case Load: op = LOAD_NAME; break;
		case Store: op = STORE_NAME; break;
		case Del: op = DELETE_NAME; break;
		}
		break;
	default:
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
	Py_ssize_t arg = compiler_add_o(c->u->u_consts, key);
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
compiler_visit_stmt(struct compiler *c, stmt_ty s) {
	Py_ssize_t i, n;

	SET_LOC(c, s);

	switch (s->kind) {
	case Expr_kind:
		return compiler_visit_stmt_expr(c, s->v.Expr.value);
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
			assert(false);
		case POP_JUMP_IF_FALSE:
		case POP_JUMP_IF_TRUE:
		case JUMP_IF_FALSE_OR_POP:
		case JUMP_IF_TRUE_OR_POP:
		case FOR_ITER:
			assert(false);
		}
	}
	return 0;
}

static int
extend_block(basicblock *bb) {
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

static PyCodeObject *
makecode(struct compiler *c, struct assembler *a, PyObject *consts)
{
	PyObject *names = NULL;
	PyObject *varnames = NULL;
	// PyObject *cellvars = NULL;

	names = dict_keys_inorder(c->u->u_names, 0);
	varnames = dict_keys_inorder(c->u->u_varnames, 0);
	if (!names || !varnames) {
		assert(false);
	}
	// cellvars = dict_keys_inorder(c->u->u_cellvars, 0);

	if (!merge_const_one(c, &names) ||
			!merge_const_one(c, &varnames)) {
	  assert(false);
	}
	assert(false);
}

static void
assemble_free(struct assembler *a) {
	Py_XDECREF(a->a_bytecode);
	Py_XDECREF(a->a_lnotab);
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
	assert(false);
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
  assert(false);
}

