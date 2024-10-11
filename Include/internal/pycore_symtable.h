#pragma once

#include "objimpl.h"
#include "listobject.h"
#include "dictobject.h"
#include "longobject.h"
#include "setobject.h"
#include "abstract.h"
#include "internal/pycore_compile.h"

#define DEF_GLOBAL 1 // global stmt
#define DEF_LOCAL 2 // assignment in code block
#define DEF_PARAM 2<<1 // formal parameter
#define DEF_NONLOCAL 2<<2 /* nonlocal stmt */
#define USE 2 << 3 // name is used
#define DEF_IMPORT 2<<6 // assignment occured via import

#define SCOPE_OFFSET 11

#define SCOPE_MASK (DEF_GLOBAL | DEF_LOCAL | DEF_PARAM | DEF_NONLOCAL)

#define DEF_BOUND (DEF_LOCAL | DEF_PARAM | DEF_IMPORT)

#define LOCAL 1
#define GLOBAL_EXPLICIT 2
#define GLOBAL_IMPLICIT 3
#define FREE 4
#define CELL 5

// defined in cpy/Python/symtable.c
// Return dummy values for now
#define LOCATION(x) \
  0, 0, 0, 0

#define VISIT_QUIT(ST, X) \
  return (X)

#define VISIT(ST, TYPE, V) \
  if (!symtable_visit_ ## TYPE((ST), (V))) \
    VISIT_QUIT((ST), 0);

#define VISIT_SEQ(ST, TYPE, SEQ) { \
  int i; \
  asdl_ ## TYPE ## _seq *seq = (SEQ); \
  for (i = 0; i < asdl_seq_LEN(seq); i++) { \
    TYPE ## _ty elt = (TYPE ## _ty) asdl_seq_GET(seq, i); \
    if (!symtable_visit_ ## TYPE((ST), elt)) \
      VISIT_QUIT((ST), 0); \
  } \
}

#define VISIT_SEQ_WITH_NULL(ST, TYPE, SEQ) { \
  int i = 0; \
  asdl_ ## TYPE ## _seq *seq = (SEQ); \
  for (i = 0; i < asdl_seq_LEN(seq); ++i) { \
    TYPE ## _ty elt = (TYPE ## _ty) asdl_seq_GET(seq, i); \
    if (!elt) continue; \
    if (!symtable_visit_ ## TYPE((ST), elt)) \
      VISIT_QUIT((ST), 0); \
  } \
}

typedef enum _block_type {
  FunctionBlock, ClassBlock, ModuleBlock, AnnotationBlock
} _Py_block_ty;

typedef struct _symtable_entry {
  PyObject_HEAD
  PyObject *ste_id; /* int: key in ste_table->st_blocks */
  struct symtable *ste_table;
  PyObject *ste_symbols; // dict: variable names to flags

  PyObject *ste_varnames; // list of function parameters

  PyObject *ste_children; // list of child blocks

  unsigned ste_comp_iter_target : 1; /* true if visiting comprehension target */
	unsigned ste_returns_value : 1; // true if namespace uses return with an argument
  _Py_block_ty ste_type; // module, class or function
} PySTEntryObject;

struct symtable {
  PyObject *st_stack;  /* list: stack of namespace info */

  struct _symtable_entry *st_cur; /* current symble table entry */
  struct _symtable_entry *st_top; /* symtable table entry for module */
  PyObject *st_private; /* name of current class of NULL */
  PyObject *st_blocks; /* dict: map AST node addresses
                          to symbol table entries */
};

static void ste_dealloc(PySTEntryObject *ste);

PyTypeObject PySTEntry_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "symtable entry",
  .tp_basicsize = sizeof(PySTEntryObject),
  .tp_flags = 0, // TODO cpy use Py_TPFLAGS_DEFAULT
	.tp_dealloc = (destructor) ste_dealloc,
};

#define PySTEntry_Check(op) Py_IS_TYPE(op, &PySTEntry_Type)

static int symtable_visit_keyword(struct symtable *st, keyword_ty k);

struct symtable *
symtable_new(void) {
	struct symtable *st;
	st = (struct symtable *) PyMem_Malloc(sizeof(struct symtable));
	if (st == NULL) {
		assert(false);
	}
  st->st_stack = PyList_New(0);
  assert(st->st_stack);

  if ((st->st_blocks = PyDict_New()) == NULL)
    assert(false);

  st->st_private = NULL;
	return st;
}

static PySTEntryObject *
ste_new(struct symtable *st, _Py_block_ty block, void *key) {
  PySTEntryObject *ste = NULL;
  PyObject *k = NULL;

  k = PyLong_FromVoidPtr(key);
  if (k == NULL)
    assert(false);

  ste = PyObject_New(PySTEntryObject, &PySTEntry_Type);
  assert(ste);

  ste->ste_table = st;
  ste->ste_id = k; /* ste owns reference to k */

	ste->ste_returns_value = 0;

  ste->ste_symbols = PyDict_New();
  ste->ste_varnames = PyList_New(0);
  ste->ste_comp_iter_target = 0;
  ste->ste_type = block;

  ste->ste_children = PyList_New(0);

  if (ste->ste_children == NULL) {
    assert(false);
  }

  if (PyDict_SetItem(st->st_blocks, ste->ste_id, (PyObject *) ste) < 0)
    assert(false);
  return ste;
}

// defined in cpy/Python/symtable.c
int symtable_enter_block(struct symtable *st, _Py_block_ty block, void *ast) {
  PySTEntryObject *prev = NULL, *ste;
  ste = ste_new(st, block, ast); // ast is used as key
  if (ste == NULL) {
    return 0;
  }
  if (PyList_Append(st->st_stack, (PyObject *) ste) < 0) {
    Py_DECREF(ste);
    return 0;
  }
	prev = st->st_cur;
	Py_DECREF(ste);
  st->st_cur = ste;

	if (prev) {
		if (PyList_Append(prev->ste_children, (PyObject *) ste) < 0) {
			return 0;
		}
	}
  return 1;
}

int symtable_exit_block(struct symtable *st) {
  Py_ssize_t size;

  st->st_cur = NULL;
  size = PyList_GET_SIZE(st->st_stack);

  if (size) {
    if (PyList_SetSlice(st->st_stack, size - 1, size, NULL) < 0)
      return 0;
    if (--size)
      st->st_cur = (PySTEntryObject *) PyList_GET_ITEM(st->st_stack, size - 1);
  }
  return 1;
}

#define SET_SCOPE(DICT, NAME, I) { \
  PyObject *o = PyLong_FromLong(I); \
  if (!o) \
    return 0; \
  if (PyDict_SetItem((DICT), (NAME), o) < 0) { \
    Py_DECREF(o); \
    return 0; \
  } \
  Py_DECREF(o); \
}

// defined in cpy/Python/symtable.c
static int
analyze_name(PySTEntryObject *ste, PyObject *scopes, PyObject *name, long flags,
    PyObject *bound, PyObject *local, PyObject *free,
    PyObject *global)
{
	printf("analyze_name %s\n", (char*) PyUnicode_DATA((PyUnicodeObject*) name));
  if (flags & DEF_GLOBAL) {
    assert(false);
  }
  if (flags & DEF_NONLOCAL) {
    assert(false);
  }
  if (flags & DEF_BOUND) {
		SET_SCOPE(scopes, name, LOCAL);
		if (PySet_Add(local, name) < 0)
			return 0;
		if (PySet_Discard(global, name) < 0)
			return 0;
		return 1;
  }
  if (bound && PySet_Contains(bound, name)) {
    assert(false);
  }
  if (global && PySet_Contains(global, name)) {
    assert(false);
  }
  SET_SCOPE(scopes, name, GLOBAL_IMPLICIT);
  return 1;
}

#undef SET_SCOPE

static int
update_symbols(PyObject *symbols, PyObject *scopes,
    PyObject *bound, PyObject *free, int classflag)
{
  PyObject *name = NULL, *itr = NULL;
  PyObject *v = NULL, *v_scope = NULL, *v_new = NULL, *v_free = NULL;
  Py_ssize_t pos = 0;

  // Update scope information for all symbols in this scope
  while (PyDict_Next(symbols, &pos, &name, &v)) {
    long scope, flags;
    assert(PyLong_Check(v));
    flags = PyLong_AS_LONG(v);
    v_scope = PyDict_GetItemWithError(scopes, name);
    assert(v_scope && PyLong_Check(v_scope));
    scope = PyLong_AS_LONG(v_scope);
    flags |= (scope << SCOPE_OFFSET);
    v_new = PyLong_FromLong(flags);
    if (!v_new)
      return 0;
    if (PyDict_SetItem(symbols, name, v_new) < 0) {
      Py_DECREF(v_new);
      return 0;
    }
    Py_DECREF(v_new);
  }

  // Record not yet resolved free variables from children (if any)
  v_free = PyLong_FromLong(FREE << SCOPE_OFFSET);
  if (!v_free)
    return 0;

  itr = PyObject_GetIter(free);
  if (itr == NULL) {
    Py_DECREF(v_free);
    return 0;
  }

  while ((name = PyIter_Next(itr))) {
    assert(false);
  }
  Py_DECREF(itr);
  Py_DECREF(v_free);
  return 1;
}
static int
analyze_block(PySTEntryObject *ste, PyObject *bound, PyObject *free,
    PyObject *global);

static int
analyze_child_block(PySTEntryObject *entry, PyObject *bound, PyObject *free,
		PyObject *global, PyObject *child_free) {
	PyObject *temp_bound = NULL, *temp_global = NULL, *temp_free = NULL;
	PyObject *temp;

	temp_bound = PySet_New(bound);
	if (!temp_bound)
		assert(false);
	temp_free = PySet_New(free);
	if (!temp_free)
		assert(false);
	temp_global = PySet_New(global);
	if (!temp_global)
		assert(false);
	
	if (!analyze_block(entry, temp_bound, temp_free, temp_global))
		assert(false);
	temp = PyNumber_InPlaceOr(child_free, temp_free);
	if (!temp)
		assert(false);
	Py_DECREF(temp);
	Py_DECREF(temp_bound);
	Py_DECREF(temp_free);
	Py_DECREF(temp_global);
	return 1;
}

static int
analyze_cells(PyObject *scopes, PyObject *free) {
	if (PySet_GET_SIZE(free) == 0) {
		return 1;
	}
	assert(false);
}

static int
analyze_block(PySTEntryObject *ste, PyObject *bound, PyObject *free,
    PyObject *global) {
  PyObject *name, *v, *local = NULL, *scopes = NULL, *newbound = NULL;
  PyObject *newglobal = NULL, *newfree = NULL, *allfree = NULL;
  PyObject *temp;
  int i, success = 0;
  Py_ssize_t pos = 0;

  local = PySet_New(NULL); // collect new names bound in block
  if (!local)
    assert(false);
  scopes = PyDict_New(); // collect scopes defined for reach name
  if (!scopes)
    assert(false);

  newglobal = PySet_New(NULL);
  if (!newglobal)
    assert(false);
  newfree = PySet_New(NULL);
  if (!newfree)
    assert(false);
  newbound = PySet_New(NULL);
  if (!newbound)
    assert(false);

  if (ste->ste_type == ClassBlock) {
    assert(false);
  }

  while (PyDict_Next(ste->ste_symbols, &pos, &name, &v)) {
    long flags = PyLong_AS_LONG(v);
    if (!analyze_name(ste, scopes, name, flags,
        bound, local, free, global))
      assert(false);
  }

  // Populate global and bound sets to be passed to children
  if (ste->ste_type != ClassBlock) {
    // Add function locals to bound set
    if (ste->ste_type == FunctionBlock) {
			temp = PyNumber_InPlaceOr(newbound, local);
			if (!temp) {
				assert(false);
			}
			Py_DECREF(temp);
    }
    // Pass down previously bound symbols
    if (bound) {
			temp = PyNumber_InPlaceOr(newbound, bound);
			if (!temp)
				assert(false);
			Py_DECREF(temp);
    }
    // Pass down known globals
    temp = PyNumber_InPlaceOr(newglobal, global);
    if (!temp) {
      assert(false);
    }
    Py_DECREF(temp);
  } else {
    // special-case __class__
    assert(false);
  }

  allfree = PySet_New(NULL);
  if (!allfree) {
    assert(false);
  }
  for (i = 0; i < PyList_GET_SIZE(ste->ste_children); ++i) {
		PyObject *c = PyList_GET_ITEM(ste->ste_children, i);
		PySTEntryObject *entry;
		assert(c && PySTEntry_Check(c));
		entry = (PySTEntryObject *) c;
		if (!analyze_child_block(entry, newbound, newfree, newglobal, allfree))
			assert(false);
  }

  temp = PyNumber_InPlaceOr(newfree, allfree);
  if (!temp) {
    assert(false);
  }
  Py_DECREF(temp);

  if (ste->ste_type == FunctionBlock && !analyze_cells(scopes, newfree)) {
    assert(false);
  }
  else if (ste->ste_type == ClassBlock) {
    assert(false);
  }
  // Records the results of the analysis in the symbol table entry
  if (!update_symbols(ste->ste_symbols, scopes, bound, newfree,
      ste->ste_type == ClassBlock))
    assert(false);

  temp = PyNumber_InPlaceOr(free, newfree);
  if (!temp) {
    assert(false);
  }
  Py_DECREF(temp);
  success = 1;
error:
  Py_XDECREF(scopes);
  Py_XDECREF(local);
  Py_XDECREF(newbound);
  Py_XDECREF(newglobal);
  Py_XDECREF(newfree);
  Py_XDECREF(allfree);
  if (!success) {
    assert(false);
  }
  return success;
}

int symtable_analyze(struct symtable *st) {
  PyObject *free, *global;
  int r;

  free = PySet_New(NULL);
  if (!free)
    return 0;
  global = PySet_New(NULL);
  if (!global) {
    Py_DECREF(free);
    return 0;
  }
  r = analyze_block(st->st_top, NULL, free, global);
  Py_DECREF(free);
  Py_DECREF(global);
  return r;
}

// define in cpy/Python/symtable.c
static int
symtable_add_def_helper(struct symtable *st, PyObject *name, int flag, struct _symtable_entry *ste,
    int lineno, int col_offset, int end_lineno, int end_col_offset) {
  PyObject *o;
  PyObject *dict;
  long val;
  PyObject *mangled = _Py_Mangle(st->st_private, name);

  if (!mangled) {
    return 0;
  }
  dict = ste->ste_symbols;
  if ((o = PyDict_GetItemWithError(dict, mangled))) {
    val = PyLong_AS_LONG(o);
    if ((flag & DEF_PARAM) && (val & DEF_PARAM)) {
      assert(false);
    }
    val |= flag;
  } else if (PyErr_Occurred()) {
    assert(false);
  } else {
    val = flag;
  }
  if (ste->ste_comp_iter_target) {
    assert(false);
  }
  o = PyLong_FromLong(val);
  if (o == NULL) {
    assert(false);
  }
  if (PyDict_SetItem(dict, mangled, o) < 0) {
    assert(false);
  }
  Py_DECREF(o);
  if (flag & DEF_PARAM) {
		if (PyList_Append(ste->ste_varnames, mangled) < 0)
			assert(false);
  } else if (flag & DEF_GLOBAL) {
    assert(false);
  }
  Py_DECREF(mangled);
  return 1;
}

static int
symtable_add_def(struct symtable *st, PyObject *name, int flag,
    int lineno, int col_offset, int end_lineno, int end_col_offset)
{
  return symtable_add_def_helper(st, name, flag, st->st_cur,
      lineno, col_offset, end_lineno, end_col_offset);
}

static int
symtable_visit_expr(struct symtable *st, expr_ty e) {
  switch (e->kind) {
  case Call_kind:
    VISIT(st, expr, e->v.Call.func);
    VISIT_SEQ(st, expr, e->v.Call.args);
    VISIT_SEQ_WITH_NULL(st, keyword, e->v.Call.keywords);
    break;
  case Name_kind:
    if (!symtable_add_def(st, e->v.Name.id,
        e->v.Name.ctx == Load ? USE : DEF_LOCAL, LOCATION(e)))
      VISIT_QUIT(st, 0);
    // TODO: add the special case for super
    break;
  case Constant_kind:
    // nothing to do here
    break;
	case BinOp_kind:
		VISIT(st, expr, e->v.BinOp.left);
		VISIT(st, expr, e->v.BinOp.right);
		break;
	case Attribute_kind:
		VISIT(st, expr, e->v.Attribute.value);
		break;
	case Compare_kind:
		VISIT(st, expr, e->v.Compare.left);
		VISIT_SEQ(st, expr, e->v.Compare.comparators);
		break;
	case Tuple_kind:
		VISIT_SEQ(st, expr, e->v.Tuple.elts);
		break;
	case List_kind:
		VISIT_SEQ(st, expr, e->v.List.elts);
		break;
  default:
    printf("Unhandled kind %d\n", e->kind);
    assert(false);
  }
  VISIT_QUIT(st, 1);
}

static int
symtable_visit_keyword(struct symtable *st, keyword_ty k) {
  VISIT(st, expr, k->value);
  return 1;
}

static int
symtable_visit_params(struct symtable *st, asdl_arg_seq *args) {
	int i;

	if (!args)
		return -1;
	
	for (i = 0; i < asdl_seq_LEN(args); i++) {
		arg_ty arg = (arg_ty) asdl_seq_GET(args, i);
		// printf("arg name '%s'\n", (char *) PyUnicode_DATA(arg->arg));
		if (!symtable_add_def(st, arg->arg, DEF_PARAM, LOCATION(arg)))
			return 0;
	}
	return 1;
}

static int
symtable_visit_arguments(struct symtable *st, arguments_ty a) {
	if (a->args && !symtable_visit_params(st, a->args))
		return 0;
	return 1;
}

static int
symtable_visit_stmt(struct symtable *st, stmt_ty s) {
  switch (s->kind) {
	case FunctionDef_kind:
		if (!symtable_add_def(st, s->v.FunctionDef.name, DEF_LOCAL, LOCATION(s)))
			VISIT_QUIT(st, 0);
		if (!symtable_enter_block(st, FunctionBlock, (void *) s))
			VISIT_QUIT(st, 0);
		VISIT(st, arguments, s->v.FunctionDef.args);
		VISIT_SEQ(st, stmt, s->v.FunctionDef.body);
		if (!symtable_exit_block(st))
			VISIT_QUIT(st, 0);
		break;
  case Expr_kind:
    VISIT(st, expr, s->v.Expr.value);
    break;
	case Assign_kind:
		VISIT_SEQ(st, expr, s->v.Assign.targets);
		VISIT(st, expr, s->v.Assign.value);
		break;
	case AugAssign_kind:
		VISIT(st, expr, s->v.AugAssign.target);
		VISIT(st, expr, s->v.AugAssign.value);
		break;
	case Return_kind:
		if (s->v.Return.value) {
			VISIT(st, expr, s->v.Return.value);
			st->st_cur->ste_returns_value = 1;
		}
		break;
	case For_kind:
		VISIT(st, expr, s->v.For.target);
		VISIT(st, expr, s->v.For.iter);
		VISIT_SEQ(st, stmt, s->v.For.body);
		if (s->v.For.orelse) {
			assert(false);
		}
		break;
	case If_kind:
		VISIT(st, expr, s->v.If.test);
		VISIT_SEQ(st, stmt, s->v.If.body);
		if (s->v.If.orelse)
			VISIT_SEQ(st, stmt, s->v.If.orelse);
		break;
	case While_kind:
		VISIT(st, expr, s->v.While.test);
		VISIT_SEQ(st, stmt, s->v.While.body);
		if (s->v.While.orelse)
			VISIT_SEQ(st, stmt, s->v.While.orelse);
		break;
	case Break_kind:
		/* nothing to do here */
		break;
  default:
    assert(false);
  }
  VISIT_QUIT(st, 1);
}

// defined in cpy/Python/symtable.c
struct symtable *
_PySymtable_Build(struct _mod *mod) {
	struct symtable *st = symtable_new();
  asdl_stmt_seq *seq;
  int i;
	if (st == NULL) {
		return NULL;
	}

  if (!symtable_enter_block(st, ModuleBlock, (void *) mod)) {
    assert(false);
  }

  st->st_top = st->st_cur;
  switch (mod->kind) {
  case Module_kind:
    seq = mod->v.Module.body;
    for (i = 0; i < asdl_seq_LEN(seq); ++i) {
      if (!symtable_visit_stmt(st,
          (stmt_ty) asdl_seq_GET(seq, i))) {
        assert(false);
      }
    }
    break;
  default:
    assert(false);
  }
  if (!symtable_exit_block(st)) {
    assert(false);
  }
  // Make the second symbol analysis pass
  if (symtable_analyze(st)) {
    return st;
  }

	// TODO: what does symtable_enter_block and symtable_exit_block do
	assert(false);
}

void
_PySymtable_Free(struct symtable *st) {
	Py_XDECREF(st->st_blocks);
	Py_XDECREF(st->st_stack);
	PyMem_Free((void *) st);
}

// defined in cpy/Python/symtable.c
PySTEntryObject *PySymtable_Lookup(struct symtable *st, void *key) {
  PyObject *k, *v;

  k = PyLong_FromVoidPtr(key);
  if (k == NULL)
    return NULL;
  v = PyDict_GetItemWithError(st->st_blocks, k);
  // printf("v is %p\n", v);

  if (v) {
    assert(PySTEntry_Check(v));
    Py_INCREF(v);
  } else if (!PyErr_Occurred()) {
    assert(false);
  }

  Py_DECREF(k);
  return (PySTEntryObject *) v;
}

static long
_PyST_GetSymbol(PySTEntryObject *ste, PyObject *name) {
  PyObject *v = PyDict_GetItemWithError(ste->ste_symbols, name);
  if (!v)
    return 0;
  assert(PyLong_Check(v));
  return PyLong_AS_LONG(v);
}

int
_PyST_GetScope(PySTEntryObject *ste, PyObject *name)
{
  long symbol = _PyST_GetSymbol(ste, name);
  return (symbol >> SCOPE_OFFSET) & SCOPE_MASK;
}

static void ste_dealloc(PySTEntryObject *ste) {
	ste->ste_table = NULL;
	Py_XDECREF(ste->ste_id);
	// Py_XDECREF(ste->ste_name);
	Py_XDECREF(ste->ste_symbols);
	Py_XDECREF(ste->ste_varnames);
	Py_XDECREF(ste->ste_children);
	// Py_XDECREF(ste->ste_directives);
	PyObject_Free(ste);
}
