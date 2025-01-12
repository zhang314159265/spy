#pragma once

#include "frameobject.h"
#include "funcobject.h"
#include "tupleobject.h"
#include "sliceobject.h"
#include "cellobject.h"
#include "genobject.h"
#include "typeobject.h"
#include "traceback.h"
#include "descrobject.h"

#include "cpython/object.h"

#define FVC_MASK 0x3
#define FVC_NONE 0x0
#define FVS_MASK 0x4
#define FVS_HAVE_SPEC 0x4

PyObject *PyObject_GetAttr(PyObject *v, PyObject *name);
#define GETLOCAL(i) (fastlocals[i])

#define SETLOCAL(i, value) do { PyObject *tmp = GETLOCAL(i); \
		GETLOCAL(i) = value; \
		Py_XDECREF(tmp); } while (0)

/* Stack manipulation macros */

#define STACK_LEVEL() ((int)(stack_pointer - f->f_valuestack))

#define BASIC_STACKADJ(n) (stack_pointer += n)
#define STACK_GROW(n) BASIC_STACKADJ(n)
#define STACK_SHRINK(n) BASIC_STACKADJ(-n)

#define JUMPTO(x) (next_instr = first_instr + (x))
#define JUMPBY(x) (next_instr += (x))

/* Code access macros */
#define INSTR_OFFSET() ((int)(next_instr - first_instr))

#define BASIC_PUSH(v) (*stack_pointer++ = (v))
#define PUSH(v) BASIC_PUSH(v)

#define BASIC_POP() (*--stack_pointer)
#define POP() BASIC_POP()
#define TOP() (stack_pointer[-1])
#define SECOND() (stack_pointer[-2])
#define THIRD() (stack_pointer[-3])
#define PEEK(n) (stack_pointer[-(n)])
#define SET_TOP(v) (stack_pointer[-1] = (v))
#define SET_SECOND(v) (stack_pointer[-2] = (v))
#define SET_THIRD(v) (stack_pointer[-3] = (v))

#define EXT_POP(STACK_POINTER) (*--(STACK_POINTER))

#define DISPATCH() goto predispatch;

#define EMPTY() (STACK_LEVEL() == 0)

#define NEXTOPARG() do { \
	_Py_CODEUNIT word = *next_instr; \
	opcode = _Py_OPCODE(word); \
	oparg = _Py_OPARG(word); \
	next_instr++; \
} while (0)

/* Tuple access macros */
#define GETITEM(v, i) PyTuple_GetItem((v), (i))

#define PREDICT_ID(op) PRED_##op
#define PREDICTED(op) PREDICT_ID(op):

// make it no-op for now
#define PREDICT(op)

#if USE_COMPUTED_GOTOS
#define TARGET(op) op: TARGET_##op
#else
#define TARGET(op) op
#endif

PyFrameObject *
_PyEval_MakeFrameVector(PyThreadState *tstate,
		PyFrameConstructor *con, PyObject *locals,
		PyObject *const *args, Py_ssize_t argcount,
		PyObject *kwnames) {
	PyCodeObject *co = (PyCodeObject*) con->fc_code;
	const Py_ssize_t total_args = co->co_argcount + co->co_kwonlyargcount;

	/* Create the frame */
	PyFrameObject *f = _PyFrame_New_NoTrack(tstate, con, locals);
	if (f == NULL) {
		return NULL;
	}
	PyObject **fastlocals = f->f_localsplus;
	PyObject **freevars = f->f_localsplus + co->co_nlocals;

	// Create a dictionary for keyword parameters (**kwargs)
	PyObject *kwdict = NULL;
  Py_ssize_t i;

  if (co->co_flags & CO_VARKEYWORDS) {
    kwdict = PyDict_New();
    if (kwdict == NULL) {
      fail(0);
    }
    i = total_args;
    if (co->co_flags & CO_VARARGS) {
      i++;
    }
    SETLOCAL(i, kwdict);
  } else {
    kwdict = NULL;
  }

	// Copy all positional arguments into local variables
	Py_ssize_t j, n;
	if (argcount > co->co_argcount) {
		n = co->co_argcount;
	} else 
	{
		n = argcount;
	}
	for (j = 0; j < n; j++) {
		PyObject *x = args[j];
		Py_INCREF(x);
		SETLOCAL(j, x);
	}

  if (co->co_flags & CO_VARARGS) {
    PyObject *u = _PyTuple_FromArray(args + n, argcount - n);
    if (u == NULL) {
      fail(0);
    }
    SETLOCAL(total_args, u);
  }

  if (kwnames != NULL) {
    Py_ssize_t kwcount = PyTuple_GET_SIZE(kwnames);
    for (i = 0; i < kwcount; i++) {
      PyObject **co_varnames;
      PyObject *keyword = PyTuple_GET_ITEM(kwnames, i);
      PyObject *value = args[i + argcount];
      Py_ssize_t j;

      if (keyword == NULL || !PyUnicode_Check(keyword)) {
        fail(0);
      }

      co_varnames = ((PyTupleObject *)(co->co_varnames))->ob_item;
      for (j = co->co_posonlyargcount; j < total_args; j++) {
        PyObject *varname = co_varnames[j];
        if (varname == keyword) {
          goto kw_found;
        }
      }

      for (j = co->co_posonlyargcount; j < total_args; j++) {
        PyObject *varname = co_varnames[j];
        int cmp = PyObject_RichCompareBool(keyword, varname, Py_EQ);
        if (cmp > 0) {
          goto kw_found;
        } else if (cmp < 0) {
          fail(0);
        }
      }

      assert(j >= total_args);
      if (kwdict == NULL) {
        fail(0);
      }

      if (PyDict_SetItem(kwdict, keyword, value) == -1) {
        fail(0);
      }
      continue;

    kw_found:
      if (GETLOCAL(j) != NULL) {
        fail(0);
      }
      Py_INCREF(value);
      SETLOCAL(j, value);
    }
  }

  if ((argcount > co->co_argcount) && !(co->co_flags & CO_VARARGS)) {
    assert(false);
  }

  if (argcount < co->co_argcount) {
    Py_ssize_t defcount = con->fc_defaults == NULL ? 0 : PyTuple_GET_SIZE(con->fc_defaults);
    printf("name %s, argcount %ld, co->co_argcount %d, defcount %ld\n", (char*) PyUnicode_DATA(con->fc_name), argcount, co->co_argcount, defcount);
    Py_ssize_t m = co->co_argcount - defcount;
    Py_ssize_t missing = 0;
    for (i = argcount; i < m; i++) {
      if (GETLOCAL(i) == NULL) {
        missing++;
      }
    }
    if (missing) {
      assert(false);
    }
    if (n > m)
      i = n - m;
    else
      i = 0;
    if (defcount) {
      PyObject **defs = &PyTuple_GET_ITEM(con->fc_defaults, 0);
      for (; i < defcount; i++) {
        if (GETLOCAL(m + i) == NULL) {
          PyObject *def = defs[i];
          Py_INCREF(def);
          printf("set local %p\n", def);
          SETLOCAL(m + i, def);
        }
      }
    }
  }

  if (co->co_kwonlyargcount > 0) {
    Py_ssize_t missing = 0;
    for (i = co->co_argcount; i < total_args; i++) {
      if (GETLOCAL(i) != NULL)
        continue;
      PyObject *varname = PyTuple_GET_ITEM(co->co_varnames, i);
      if (con->fc_kwdefaults != NULL) {
        PyObject *def = PyDict_GetItemWithError(con->fc_kwdefaults, varname);
        if (def) {
          Py_INCREF(def);
          SETLOCAL(i, def);
          continue;
        } else if (_PyErr_Occurred(tstate)) {
          assert(false);
        }
      }
      missing++;
    }
    if (missing) {
      assert(false);
    }
  }

  // printf("#cell %ld, #free %ld\n", PyTuple_GET_SIZE(co->co_cellvars), PyTuple_GET_SIZE(co->co_freevars));

  for (i = 0; i < PyTuple_GET_SIZE(co->co_cellvars); ++i) {
    PyObject *c;
    Py_ssize_t arg;

    if (co->co_cell2arg != NULL &&
        (arg = co->co_cell2arg[i]) != CO_CELL_NOT_AN_ARG) {
      c = PyCell_New(GETLOCAL(arg));
      SETLOCAL(arg, NULL);
    } else {
      c = PyCell_New(NULL);
    }
    if (c == NULL)
      assert(false);
    SETLOCAL(co->co_nlocals + i, c);
  }

  for (i = 0; i < PyTuple_GET_SIZE(co->co_freevars); ++i) {
    PyObject *o = PyTuple_GET_ITEM(con->fc_closure, i);
    Py_INCREF(o);
    freevars[PyTuple_GET_SIZE(co->co_cellvars) + i] = o;
  }

	return f;
}

static inline PyObject *
_PyEval_EvalFrame(PyThreadState *tstate, PyFrameObject *f, int throwflag) {
	return tstate->interp->eval_frame(tstate, f, throwflag);
}

PyObject *
call_function(PyThreadState *tstate,
		PyObject ***pp_stack,
		Py_ssize_t oparg,
		PyObject *kwnames) {
	PyObject **pfunc = (*pp_stack) - oparg - 1;
	PyObject *func = *pfunc;
	PyObject *x, *w;
	Py_ssize_t nkwargs = (kwnames == NULL) ? 0 : PyTuple_GET_SIZE(kwnames);
	Py_ssize_t nargs = oparg - nkwargs;
	PyObject **stack = (*pp_stack) - oparg;

	x = PyObject_Vectorcall(func, stack, nargs | PY_VECTORCALL_ARGUMENTS_OFFSET, kwnames);
	assert((x != NULL) ^ (_PyErr_Occurred(tstate) != NULL));

	while ((*pp_stack) > pfunc) {
		w = EXT_POP(*pp_stack);
		Py_DECREF(w);
	}
	return x;
}

static void dump_value(PyObject *obj) {
  if (!obj) {
    printf("NULL\n");
    return;
  }
  if (obj == Py_None) {
    printf("none\n");
    return;
  }
  const char *tpstr = obj->ob_type->tp_name;
  if (
      strcmp(tpstr, "ModuleSpec") == 0 ||
      strcmp(tpstr, "_thread.lock") == 0 ||
      strcmp(tpstr, "tuple") == 0 ||
      strcmp(tpstr, "_io.BufferedReader") == 0 ||
      strcmp(tpstr, "property") == 0 ||
      strcmp(tpstr, "SourceFileLoader") == 0 ||
      strcmp(tpstr, "types.SimpleNamespace") == 0 ||
      strcmp(tpstr, "dict") == 0 ||
      // strcmp(tpstr, "set") == 0 ||
      strcmp(tpstr, "generator") == 0 ||
      strcmp(tpstr, "sys.flags") == 0 ||
      strcmp(tpstr, "_ModuleLockManager") == 0 ||
      strcmp(tpstr, "_ModuleLock") == 0 ||
      strcmp(tpstr, "object") == 0 ||
      strcmp(tpstr, "type") == 0 ||
      strcmp(tpstr, "cell") == 0 ||
      strcmp(tpstr, "method") == 0 ||
      strcmp(tpstr, "code") == 0 ||
      strcmp(tpstr, "list_iterator") == 0 ||
      strcmp(tpstr, "str_iterator") == 0 ||
      strcmp(tpstr, "FileFinder") == 0 ||
      strcmp(tpstr, "tuple_iterator") == 0 ||
      strcmp(tpstr, "method_descriptor") == 0 ||

      // structseq
      strcmp(tpstr, "stat_result") == 0 ||
      0
  ) {
    printf("type %s\n", tpstr);
    return;
  }
  printf("type %s, tostr %s\n", obj->ob_type->tp_name, (char *) PyUnicode_DATA(PyObject_Str(obj)));
}

void debug_eval_done_frame(PyFrameObject *f, PyObject *retval) {
  PyCodeObject *co = f->f_code;
  PyObject *name = co->co_name;
  printf("\033[34mReturn from frame %s\033[0m\n", (char *) PyUnicode_DATA(name));
  printf("  RETVAL: "); dump_value(retval);
}

void debug_eval(int opcode, int oparg,
    PyObject **fastlocals,
    PyObject *names,
    PyObject **stack_pointer) {
  printf("\033[32m<opcode %s (%d), oparg %d>\033[0m\n", opcode_to_str(opcode), opcode, oparg);

  switch (opcode) {
  case LOAD_FAST: {
    PyObject *value = GETLOCAL(oparg);
    dump_value(value);
    break;
  }
  case LOAD_ATTR: {
    PyObject *name = GETITEM(names, oparg);
    printf("attr name %s\n", (char*) PyUnicode_DATA(name));
    PyObject *owner = TOP();
    printf("owner "); dump_value(owner);
    if (PyObject_HasAttr(owner, name)) {
      PyObject *res = PyObject_GetAttr(owner, name);
      printf("res "); dump_value(res);
    } else {
      printf("attr not found\n");
    }
    break;
  }
  case STORE_ATTR: {
    PyObject *name = GETITEM(names, oparg);
    PyObject *owner = TOP();
    PyObject *v = SECOND();
    printf("attr name %s\n", (char *) PyUnicode_DATA(name));
    printf("owner "); dump_value(owner);
    printf("newval "); dump_value(v);
    break;
  }
  case LOAD_GLOBAL: {
    PyObject *name = GETITEM(names, oparg);
    printf("gvar name %s\n", (char *) PyUnicode_DATA(name));
    break; 
  }
  case LOAD_METHOD: {
    PyObject *name = GETITEM(names, oparg);
    printf("method name %s\n", (char *) PyUnicode_DATA(name));
    break; 
  }
  case CALL_FUNCTION: {
    PyObject **itr = stack_pointer - oparg - 1;
    PyObject *func = *itr++;
    printf("  func: "); dump_value(func);
    for (int i = 0; i < oparg; ++i) {
      printf("  ARG %d: ", i);
      dump_value(*itr++); 
    }
    break;
  }
  case CALL_METHOD: {
    PyObject *meth = PEEK(oparg + 2);
    if (meth == NULL) {
      printf("  no self\n");
      PyObject **itr = stack_pointer - oparg - 1;
      PyObject *func = *itr++;
      printf("  func: "); dump_value(func);
      
      for (int i = 0; i < oparg; ++i) {
        printf("  ARG %d: ", i);
        dump_value(*itr++); 
      }
    } else {
      PyObject **itr = stack_pointer - oparg - 1;
      printf("  with self\n");
      printf("  meth: "); dump_value(meth);
      for (int i = 0; i < oparg + 1; ++i) {
        printf("  ARG %d: ", i);
        dump_value(*itr++); 
      }
    }
    break;
  }
  default:
    break;
  }
}

static PyObject *
unicode_concatenate(PyThreadState *tstate, PyObject *v, PyObject *w,
    PyFrameObject *f, const _Py_CODEUNIT *next_instr) {
  PyObject *res;
  if (Py_REFCNT(v) == 2) {
    int opcode, oparg;
    NEXTOPARG();
    switch (opcode) {
    case STORE_FAST:
    {
      PyObject **fastlocals = f->f_localsplus;
      if (GETLOCAL(oparg) == v)
        SETLOCAL(oparg, NULL);
      break;
    }
    case STORE_DEREF:
    case STORE_NAME:
      fail("opcode is %d\n", opcode);
    }
  }
  res = v;
  PyUnicode_Append(&res, w);
  return res;
}

static PyObject *
special_lookup(PyThreadState *tstate, PyObject *o, _Py_Identifier *id) {
  PyObject *res;
  res = _PyObject_LookupSpecial(o, id);
  if (res == NULL && !_PyErr_Occurred(tstate)) {
    _PyErr_SetObject(tstate, PyExc_AttributeError, _PyUnicode_FromId(id));
    return NULL;
  }
  return res;
}

PyObject *PyImport_ImportModuleLevelObject(PyObject *name, PyObject *globals,
		PyObject *locals, PyObject *fromlist,
		int level);

static PyObject *
import_name(PyThreadState *tstate, PyFrameObject *f,
    PyObject *name, PyObject *fromlist, PyObject *level) {
  _Py_IDENTIFIER(__import__);
  PyObject *import_func, *res;

  import_func = _PyDict_GetItemIdWithError(f->f_builtins, &PyId___import__);
  if (import_func == NULL) {
    assert(false);
  }
  if (import_func == tstate->interp->import_func) {
    int ilevel = _PyLong_AsInt(level);
    if (ilevel == -1 && _PyErr_Occurred(tstate)) {
      return NULL;
    }
    res = PyImport_ImportModuleLevelObject(
      name,
      f->f_globals,
      f->f_locals == NULL ? Py_None : f->f_locals,
      fromlist,
      ilevel
    );
    return res;
  }
  assert(false);
}

static int
do_raise(PyThreadState *tstate, PyObject *exc, PyObject *cause) {
  PyObject *type = NULL, *value = NULL;

  if (exc == NULL) {
    fail(0);
  }

  if (PyExceptionClass_Check(exc)) {
    fail(0);
  } else if (PyExceptionInstance_Check(exc)) {
    value = exc;
    type = PyExceptionInstance_Class(exc);
    Py_INCREF(type);
  } else {
    fail(0);
  }

  assert(type != NULL);
  assert(value != NULL);

  if (cause) {
    fail(0);
  }

  _PyErr_SetObject(tstate, type, value);
  Py_DECREF(value);
  Py_DECREF(type);
  return 0;
}

// TODO follow cpy
#define C_TRACE(x, call) \
  x = call; 

static PyObject *
do_call_core(PyThreadState *tstate,
    PyObject *func,
    PyObject *callargs,
    PyObject *kwdict) {
  PyObject *result;
  if (PyCFunction_CheckExact(func) || PyCMethod_CheckExact(func)) {
    result = PyObject_Call(func, callargs, kwdict);
    return result;
  } else if (Py_IS_TYPE(func, &PyMethodDescr_Type)) {
    fail(0);
  }
  return PyObject_Call(func, callargs, kwdict);
}

static int
check_args_iterable(PyThreadState *tstate, PyObject *func, PyObject *args) {
  if (Py_TYPE(args)->tp_iter == NULL && !PySequence_Check(args)) {
    fail(0);
  }
  return 0;
}

static PyObject *
import_from(PyThreadState *tstate, PyObject *v, PyObject *name) {
  PyObject *x;

  if (_PyObject_LookupAttr(v, name, &x) != 0) {
    return x;
  }
  fail(0);
}

#define UNWIND_BLOCK(b) \
  while (STACK_LEVEL() > (b)->b_level) { \
    PyObject *v = POP(); \
    Py_XDECREF(v); \
  }

#define UNWIND_EXCEPT_HANDLER(b) \
  do { \
    PyObject *type, *value, *traceback; \
    _PyErr_StackItem *exc_info; \
    assert(STACK_LEVEL() >= (b)->b_level + 3); \
    while (STACK_LEVEL() > (b)->b_level + 3) { \
      value = POP(); \
      Py_XDECREF(value); \
    } \
    exc_info = tstate->exc_info; \
    type = exc_info->exc_type; \
    value = exc_info->exc_value; \
    traceback = exc_info->exc_traceback; \
    exc_info->exc_type = POP(); \
    exc_info->exc_value = POP(); \
    exc_info->exc_traceback = POP(); \
    Py_XDECREF(type); \
    Py_XDECREF(value); \
    Py_XDECREF(traceback); \
  } while (0)

PyObject *
_PyEval_EvalFrameDefault(PyThreadState *tstate, PyFrameObject *f, int throwflag) {
	PyObject **stack_pointer;
	const _Py_CODEUNIT *next_instr;
	int opcode;
	int oparg;
	PyObject **fastlocals, **freevars;
	const _Py_CODEUNIT *first_instr;
	PyObject *retval = NULL;
	PyCodeObject *co;

	PyObject *names;
	PyObject *consts;

	/* push frame */
	tstate->frame = f;
	co = f->f_code;

	names = co->co_names;
	consts = co->co_consts;
	fastlocals = f->f_localsplus;
  freevars = f->f_localsplus + co->co_nlocals;

	first_instr = (_Py_CODEUNIT *) PyBytes_AS_STRING(co->co_code);
	assert(f->f_lasti >= -1);
	next_instr = first_instr + f->f_lasti + 1;
	stack_pointer = f->f_valuestack + f->f_stackdepth;
	f->f_stackdepth = -1;
	f->f_state = FRAME_EXECUTING;

	if (throwflag) {
		assert(false);
	}

main_loop:
	for (;;) {
		assert(stack_pointer >= f->f_valuestack); // else underflow
		assert(STACK_LEVEL() <= co->co_stacksize); // else overflow
		// assert(!_PyErr_Occurred(tstate));

		tracing_dispatch:
		{
			int instr_prev = f->f_lasti;
			f->f_lasti = INSTR_OFFSET();
			NEXTOPARG();
		}

		goto dispatch_opcode;

		predispatch:
			f->f_lasti = INSTR_OFFSET();
			NEXTOPARG();

		dispatch_opcode:

    if (getenv("DEBUG_EVAL")) {
      debug_eval(opcode, oparg, fastlocals, names, stack_pointer);
    }

		switch (opcode) {
		case TARGET(LOAD_METHOD): {
			PyObject *name = GETITEM(names, oparg);
			PyObject *obj = TOP();
			PyObject *meth = NULL;

			int meth_found = _PyObject_GetMethod(obj, name, &meth);

			if (meth == NULL) {
        goto error;
			}

			if (meth_found) {
				SET_TOP(meth);
				PUSH(obj); // self
			} else {
        SET_TOP(NULL);
        Py_DECREF(obj);
        PUSH(meth);
			}
			DISPATCH();
		}
    case TARGET(JUMP_IF_TRUE_OR_POP): {
      PyObject *cond = TOP();
      int err;
      if (Py_IsFalse(cond)) {
        STACK_SHRINK(1);
        Py_DECREF(cond);
        DISPATCH();
      }
      if (Py_IsTrue(cond)) {
        JUMPTO(oparg);
        DISPATCH();
      }
      err = PyObject_IsTrue(cond);
      if (err > 0) {
        JUMPTO(oparg);
      } else if (err == 0) {
        STACK_SHRINK(1);
        Py_DECREF(cond);
      } else {
        goto error;
      }
      DISPATCH();
    }
    case TARGET(STORE_DEREF): {
      PyObject *v = POP();
      PyObject *cell = freevars[oparg];
      PyObject *oldobj = PyCell_GET(cell);
      PyCell_SET(cell, v);
      Py_XDECREF(oldobj);
      DISPATCH();
    }
    case TARGET(BUILD_STRING): {
      PyObject *str;
      PyObject *empty = PyUnicode_New(0, 0);
      if (empty == NULL) {
        goto error;
      }
      PyObject * _PyUnicode_JoinArray(PyObject *separator, PyObject *const *items, Py_ssize_t seqlen);
      str = _PyUnicode_JoinArray(empty, stack_pointer - oparg, oparg);

      Py_DECREF(empty);
      if (str == NULL)
        goto error;
      while (--oparg >= 0) {
        PyObject *item = POP();
        Py_DECREF(item);
      }
      PUSH(str);
      DISPATCH();
    }
    case TARGET(FORMAT_VALUE): {
      // Handles f-string value formatting
      PyObject *result;
      PyObject *fmt_spec;
      PyObject *value;
      PyObject *(*conv_fn)(PyObject *);
      int which_conversion = oparg & FVC_MASK;
      int have_fmt_spec = (oparg & FVS_MASK) == FVS_HAVE_SPEC;

      fmt_spec = have_fmt_spec ? POP() : NULL;
      value = POP();

      switch (which_conversion) {
      case FVC_NONE: conv_fn = NULL; break;
      default:
        fail("conversion is %d\n", which_conversion);
        goto error;
      }
      if (conv_fn != NULL) {
        fail(0);
      }

      if (PyUnicode_CheckExact(value) && fmt_spec == NULL) {
        result = value;
      } else {
        fail(0);
      }

      PUSH(result);
      DISPATCH();
    }
    case TARGET(WITH_EXCEPT_START): {
      PyObject *exit_func;
      PyObject *exc, *val, *tb, *res;

      exc = TOP();
      val = SECOND();
      tb = THIRD();
      assert(!Py_IsNone(exc));
      assert(!PyLong_Check(exc));
      exit_func = PEEK(7);
      PyObject *stack[4] = {NULL, exc, val, tb};
      res = PyObject_Vectorcall(exit_func, stack + 1,
          3 | PY_VECTORCALL_ARGUMENTS_OFFSET, NULL);
      if (res == NULL)
        goto error;
      PUSH(res);
      DISPATCH();
    }
    case TARGET(ROT_TWO): {
      PyObject *top = TOP();
      PyObject *second = SECOND();
      SET_TOP(second);
      SET_SECOND(top);
      DISPATCH();
    }
    case TARGET(DELETE_SUBSCR): {
      PyObject *sub = TOP();
      PyObject *container = SECOND();
      int err;
      STACK_SHRINK(2);
      err = PyObject_DelItem(container, sub);
      Py_DECREF(container);
      Py_DECREF(sub);
      if (err != 0)
        goto error;
      DISPATCH();
    }
    case TARGET(RERAISE): {
      void _PyErr_Restore(PyThreadState *tstate, PyObject *type, PyObject *value, PyObject *traceback);
      assert(f->f_iblock > 0);
      if (oparg) {
        f->f_lasti = f->f_blockstack[f->f_iblock - 1].b_handler;
      }
      PyObject *exc = POP();
      PyObject *val = POP();
      PyObject *tb = POP();
      assert(PyExceptionClass_Check(exc));
      _PyErr_Restore(tstate, exc, val, tb);
      goto exception_unwind;
    }
    case TARGET(CALL_FUNCTION_EX): {
      PyObject *func, *callargs, *kwargs = NULL, *result;
      if (oparg & 0x01) {
        kwargs = POP();
        if (!PyDict_CheckExact(kwargs)) {
          fail(0);
        }
        assert(PyDict_CheckExact(kwargs));
      }
      callargs = POP();
      func = TOP();
      if (!PyTuple_CheckExact(callargs)) {
        if (check_args_iterable(tstate, func, callargs) < 0) {
          fail(0);
        }
        Py_SETREF(callargs, PySequence_Tuple(callargs));
        if (callargs == NULL) {
          fail(0);
        }
      }
      assert(PyTuple_CheckExact(callargs));

      result = do_call_core(tstate, func, callargs, kwargs);

      Py_DECREF(func);
      Py_DECREF(callargs);
      Py_XDECREF(kwargs);

      SET_TOP(result);
      if (result == NULL) {
        goto error;
      }
      DISPATCH();
    }

    case TARGET(DICT_MERGE): {
      PyObject *update = POP();
      PyObject *dict = PEEK(oparg);

      if (_PyDict_MergeEx(dict, update, 2) < 0) {
        fail(0);
      }
      Py_DECREF(update);
      DISPATCH();
    }
    case TARGET(RAISE_VARARGS): {
      PyObject *cause = NULL, *exc = NULL;
      switch (oparg) {
      case 2:
        cause = POP();
      case 1:
        exc = POP();
      case 0:
        if (do_raise(tstate, exc, cause)) {
          fail(0);
        }
        break;
      default:
        fail(0);
      }
      goto error;
    }
    case TARGET(CALL_FUNCTION_KW): {
      PyObject **sp, *res, *names;

      names = POP();
      assert(PyTuple_Check(names));
      assert(PyTuple_GET_SIZE(names) <= oparg);
      sp = stack_pointer;
      res = call_function(tstate, &sp, oparg, names);
      stack_pointer = sp;
      PUSH(res);
      Py_DECREF(names);

      if (res == NULL) {
        goto error;
      }
      DISPATCH();
    }
    case TARGET(JUMP_IF_NOT_EXC_MATCH): {
      PyObject *right = POP();
      PyObject *left = POP();
      if (PyTuple_Check(right)) {
        Py_ssize_t i, length;
        length = PyTuple_GET_SIZE(right);
        for (i = 0; i < length; i++) {
          PyObject *exc = PyTuple_GET_ITEM(right, i);
          if (!PyExceptionClass_Check(exc)) {
            fail(0);
          }
        }
      } else {
        if (!PyExceptionClass_Check(right)) {
          fail(0);
        }
      }
      int res = PyErr_GivenExceptionMatches(left, right);
      Py_DECREF(left);
      Py_DECREF(right);
      if (res > 0) {
      } else if (res == 0) {
        JUMPTO(oparg);
      } else {
        fail(0);
      }
      DISPATCH();
    }
    case TARGET(POP_EXCEPT): {
      PyObject *type, *value, *traceback;
      _PyErr_StackItem *exc_info;
      PyTryBlock *b = PyFrame_BlockPop(f);
      if (b->b_type != EXCEPT_HANDLER) {
        fail(0);
      }
      assert(STACK_LEVEL() >= (b)->b_level + 3 &&
        STACK_LEVEL() <= (b)->b_level + 4);
      exc_info = tstate->exc_info;
      type = exc_info->exc_type;
      value = exc_info->exc_value;
      traceback = exc_info->exc_traceback;
      exc_info->exc_type = POP();
      exc_info->exc_value = POP();
      exc_info->exc_traceback = POP();
      Py_XDECREF(type);
      Py_XDECREF(value);
      Py_XDECREF(traceback);
      DISPATCH();
    }
    case TARGET(SETUP_FINALLY): {
      PyFrame_BlockSetup(f, SETUP_FINALLY, INSTR_OFFSET() + oparg,
          STACK_LEVEL());
      DISPATCH();
    }
    case TARGET(BUILD_CONST_KEY_MAP): {
      Py_ssize_t i;
      PyObject *map;
      PyObject *keys = TOP();
      if (!PyTuple_CheckExact(keys) ||
        PyTuple_GET_SIZE(keys) != (Py_ssize_t) oparg) {
        assert(false);
      }
      map = _PyDict_NewPresized((Py_ssize_t) oparg);
      if (map == NULL) {
        goto error;
      }
      for (i = oparg; i > 0; i--) {
        int err;
        PyObject *key = PyTuple_GET_ITEM(keys, oparg - i);
        PyObject *value = PEEK(i + 1);
        err = PyDict_SetItem(map, key, value);
        if (err != 0) {
          assert(false);
        }
      }
      Py_DECREF(POP());
      while (oparg--) {
        Py_DECREF(POP());
      }
      PUSH(map);
      DISPATCH();
    }
    case TARGET(STORE_GLOBAL): {
      PyObject *name = GETITEM(names, oparg);
      PyObject *v = POP();
      int err;
      err = PyDict_SetItem(f->f_globals, name, v);
      Py_DECREF(v);
      if (err != 0)
        goto error;
      DISPATCH();
    }
    case TARGET(IMPORT_NAME): {
      PyObject *name = GETITEM(names, oparg);
      PyObject *fromlist = POP();
      PyObject *level = TOP();
      PyObject *res;
      res = import_name(tstate, f, name, fromlist, level);
      Py_DECREF(level);
      Py_DECREF(fromlist);
      SET_TOP(res);
      if (res == NULL)
        goto error;
      DISPATCH();
    }
    case TARGET(IMPORT_FROM): {
      PyObject *name = GETITEM(names, oparg);
      PyObject *from = TOP();
      PyObject *res;
      res = import_from(tstate, from, name);
      PUSH(res);
      if (res == NULL)
        goto error;
      DISPATCH();
    }
    case TARGET(NOP): {
      DISPATCH();
    }
    case TARGET(LOAD_DEREF): {
      PyObject *cell = freevars[oparg];
      // printf("LOAD_DEREF cell type %s\n", Py_TYPE(cell)->tp_name);
      assert(PyCell_Check(cell));
      PyObject *value = PyCell_GET(cell);
      if (value == NULL) {
        assert(false);
      }
      Py_INCREF(value);
      PUSH(value);
      DISPATCH();
    }
    case TARGET(GEN_START): {
      PyObject *none = POP();
      assert(none == Py_None);
      assert(oparg < 3);
      Py_DECREF(none);
      DISPATCH();
    }
    case TARGET(YIELD_VALUE): {
      retval = POP();
      if (co->co_flags & CO_ASYNC_GENERATOR) {
        assert(false);
      }
      f->f_state = FRAME_SUSPENDED;
      f->f_stackdepth = (int)(stack_pointer - f->f_valuestack);
      goto exiting;
    }
    case TARGET(STORE_ATTR): {
      PyObject *name = GETITEM(names, oparg);
      PyObject *owner = TOP();
      PyObject *v = SECOND();
      int err;
      STACK_SHRINK(2);
      err = PyObject_SetAttr(owner, name, v);
      Py_DECREF(v);
      Py_DECREF(owner);
      if (err != 0)
        goto error;
      DISPATCH();
    }
    case TARGET(LOAD_BUILD_CLASS): {
      _Py_IDENTIFIER(__build_class__);

      PyObject *bc;
      if (PyDict_CheckExact(f->f_builtins)) {
        bc = _PyDict_GetItemIdWithError(f->f_builtins, &PyId___build_class__);
        if (bc == NULL) {
          assert(false);
        }
        Py_INCREF(bc);
      } else {
        assert(false);
      }
      PUSH(bc);
      DISPATCH();
    }
		case TARGET(DICT_UPDATE): {
			PyObject *update = POP();
			PyObject *dict = PEEK(oparg);
			if (PyDict_Update(dict, update) < 0) {
				assert(false);
			}
			Py_DECREF(update);
			DISPATCH();
		}
		case TARGET(SET_ADD): {
			PyObject *v = POP();
			PyObject *set = PEEK(oparg);
			int err;
			err = PySet_Add(set, v);
			Py_DECREF(v);
			if (err != 0)
				goto error;
			DISPATCH();
		}
		case TARGET(SET_UPDATE): {
			PyObject *iterable = POP();
			PyObject *set = PEEK(oparg);
			int err = _PySet_Update(set, iterable);
			Py_DECREF(iterable);
			if (err < 0) {
				goto error;
			}
			DISPATCH();
		}

		case TARGET(ROT_THREE): {
			PyObject *top = TOP();
			PyObject *second = SECOND();
			PyObject *third = THIRD();
			SET_TOP(second);
			SET_SECOND(third);
			SET_THIRD(top);
			DISPATCH();
		}

		case TARGET(DUP_TOP_TWO): {
			PyObject *top = TOP();
			PyObject *second = SECOND();
			Py_INCREF(top);
			Py_INCREF(second);
			STACK_GROW(2);
			SET_TOP(top);
			SET_SECOND(second);
			DISPATCH();
		}

		case TARGET(DUP_TOP): {
			PyObject *top = TOP();
			Py_INCREF(top);
			PUSH(top);
			DISPATCH();
		}

		case TARGET(UNPACK_SEQUENCE): {
			PyObject *seq = POP(), *item, **items;
			if (PyTuple_CheckExact(seq) && PyTuple_GET_SIZE(seq) == oparg) {
        items = ((PyTupleObject *) seq)->ob_item;
        while (oparg--) {
          item = items[oparg];
          Py_INCREF(item);
          PUSH(item);
        }
			} else if (PyList_CheckExact(seq) &&
					PyList_GET_SIZE(seq) == oparg) {
				items = ((PyListObject *) seq)->ob_item;
				while (oparg--) {
					item = items[oparg];
					Py_INCREF(item);
					PUSH(item);
				}
			} else {
				assert(false);
			}
			Py_DECREF(seq);
			DISPATCH();
		}

		case TARGET(BUILD_SLICE): {
			PyObject *start, *stop, *step, *slice;
			if (oparg == 3)
				step = POP();
			else
				step = NULL;
			stop = POP();
			start = TOP();
			slice = PySlice_New(start, stop, step);
			Py_DECREF(start);
			Py_DECREF(stop);
			Py_XDECREF(step);
			SET_TOP(slice);
			if (slice == NULL)
				goto error;
			DISPATCH();
		}

#define HANDLE_BINARY_OP(meth) \
		  PyObject *right = POP(); \
			PyObject *left = TOP(); \
			PyObject *res = meth(left, right); \
			Py_DECREF(left); \
			Py_DECREF(right); \
			SET_TOP(res); \
			if (res == NULL) \
				goto error; \
			DISPATCH()

		case TARGET(INPLACE_SUBTRACT): {
			HANDLE_BINARY_OP(PyNumber_InPlaceSubtract);
		}
		case TARGET(INPLACE_FLOOR_DIVIDE): {
			HANDLE_BINARY_OP(PyNumber_InPlaceFloorDivide);
		}
		case TARGET(INPLACE_TRUE_DIVIDE): {
			HANDLE_BINARY_OP(PyNumber_InPlaceTrueDivide);
		}
		case TARGET(INPLACE_MULTIPLY): {
			HANDLE_BINARY_OP(PyNumber_InPlaceMultiply);
		}
		case TARGET(INPLACE_MODULO): {
			HANDLE_BINARY_OP(PyNumber_InPlaceRemainder);
		}
		case TARGET(INPLACE_POWER): {
		  PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *res = PyNumber_InPlacePower(left, right, Py_None);
			Py_DECREF(left);
			Py_DECREF(right);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH()
		}
		case TARGET(INPLACE_LSHIFT): {
			HANDLE_BINARY_OP(PyNumber_InPlaceLshift);
		}
		case TARGET(INPLACE_RSHIFT): {
			HANDLE_BINARY_OP(PyNumber_InPlaceRshift);
		}
		case TARGET(INPLACE_AND): {
			HANDLE_BINARY_OP(PyNumber_InPlaceAnd);
		}
		case TARGET(INPLACE_XOR): {
			HANDLE_BINARY_OP(PyNumber_InPlaceXor);
		}
		case TARGET(INPLACE_OR): {
			HANDLE_BINARY_OP(PyNumber_InPlaceOr);
		}
		case TARGET(IS_OP): {
			PyObject *right = POP();
			PyObject *left = TOP();
			int res = Py_Is(left, right) ^ oparg;
			PyObject *b = res ? Py_True : Py_False;
			Py_INCREF(b);
			SET_TOP(b);
			Py_DECREF(left);
			Py_DECREF(right);
			DISPATCH();
		}
		case TARGET(CONTAINS_OP): {
			PyObject *right = POP();
			PyObject *left = POP();
			int res = PySequence_Contains(right, left);
			Py_DECREF(left);
			Py_DECREF(right);
			if (res < 0) {
				goto error;
			}
			PyObject *b = (res ^ oparg) ? Py_True : Py_False;
			Py_INCREF(b);
			PUSH(b);
			DISPATCH();
		}

		case TARGET(UNARY_NOT): {
			PyObject *value = TOP();
			int err = PyObject_IsTrue(value);
			Py_DECREF(value);
			if (err == 0) {
				Py_INCREF(Py_True);
				SET_TOP(Py_True);
				DISPATCH();
			} else if (err > 0) {
				Py_INCREF(Py_False);
				SET_TOP(Py_False);
				DISPATCH();
			}
			STACK_SHRINK(1);
			goto error;
		}
		case TARGET(UNARY_POSITIVE): {
			PyObject *value = TOP();
			PyObject *res = PyNumber_Positive(value);
			Py_DECREF(value);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(UNARY_NEGATIVE): {
			PyObject *value = TOP();
			PyObject *res = PyNumber_Negative(value);
			Py_DECREF(value);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(UNARY_INVERT): {
			PyObject *value = TOP();
			PyObject *res = PyNumber_Invert(value);
			Py_DECREF(value);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(BINARY_LSHIFT): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *res = PyNumber_Lshift(left, right);
			Py_DECREF(left);
			Py_DECREF(right);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(BINARY_RSHIFT): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *res = PyNumber_Rshift(left, right);
			Py_DECREF(left);
			Py_DECREF(right);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(BINARY_XOR): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *res = PyNumber_Xor(left, right);
			Py_DECREF(left);
			Py_DECREF(right);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(BINARY_OR): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *res = PyNumber_Or(left, right);
			Py_DECREF(left);
			Py_DECREF(right);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(BINARY_AND): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *res = PyNumber_And(left, right);
			Py_DECREF(left);
			Py_DECREF(right);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(BUILD_SET): {
			PyObject *set = PySet_New(NULL);
			int err = 0;
			int i;

			if (set == NULL)
				goto error;
			for (i = oparg; i > 0; i--) {
				PyObject *item = PEEK(i);
				if (err == 0)
					err = PySet_Add(set, item);
				Py_DECREF(item);
			}
			STACK_SHRINK(oparg);
			if (err != 0) {
				Py_DECREF(set);
				goto error;
			}
			PUSH(set);
			DISPATCH();
		}
		case TARGET(BINARY_SUBSCR): {
			PyObject *sub = POP();
			PyObject *container = TOP();
			PyObject *res = PyObject_GetItem(container, sub);
			Py_DECREF(container);
			Py_DECREF(sub);
			SET_TOP(res);
			if (res == NULL)
				goto error;
			DISPATCH();
		}
		case TARGET(STORE_SUBSCR): {
			PyObject *sub = TOP();
			PyObject *container = SECOND();
			PyObject *v = THIRD();
			int err;
			STACK_SHRINK(3);
			// container[sub] = v
			err = PyObject_SetItem(container, sub, v);
			Py_DECREF(v);
			Py_DECREF(container);
			Py_DECREF(sub);
			if (err != 0)
				goto error;
			DISPATCH();
		}
		case TARGET(BUILD_MAP): {
			Py_ssize_t i;
			PyObject *map = _PyDict_NewPresized((Py_ssize_t) oparg);
			if (map == NULL)
				goto error;
			for (i = oparg; i > 0; i--) {
				int err;
				PyObject *key = PEEK(2 * i);
				PyObject *value = PEEK(2 * i - 1);
				err = PyDict_SetItem(map, key, value);
				if (err != 0) {
					Py_DECREF(map);
					goto error;
				}
			}
			while (oparg--) {
				Py_DECREF(POP());
				Py_DECREF(POP());
			}
			PUSH(map);
			DISPATCH();
		}
		case TARGET(LIST_TO_TUPLE): {
			PyObject *list = POP();
			PyObject *tuple = PyList_AsTuple(list);
			Py_DECREF(list);
			if (tuple == NULL) {
				goto error;
			}
			PUSH(tuple);
			DISPATCH();
		}
		case TARGET(LIST_EXTEND): {
			PyObject *iterable = POP();
			PyObject *list = PEEK(oparg);
			PyObject *none_val = _PyList_Extend((PyListObject *) list, iterable);
			if (none_val == NULL) {
				assert(false);
			}
			Py_DECREF(none_val);
			Py_DECREF(iterable);
			DISPATCH();
		}
		case TARGET(LIST_APPEND): {
			PyObject *v = POP();
			PyObject *list = PEEK(oparg);
			int err;
			err = PyList_Append(list, v);
			Py_DECREF(v);
			if (err != 0)
				assert(false);
			DISPATCH();
		}
		case TARGET(BUILD_LIST): {
			PyObject *list = PyList_New(oparg);
			if (list == NULL)
				assert(false);
			while (--oparg >= 0) {
				PyObject *item = POP();
				PyList_SET_ITEM(list, oparg, item);
			}
			PUSH(list);
			DISPATCH();
		}
		case TARGET(BUILD_TUPLE): {
			PyObject *tup = PyTuple_New(oparg);
			if (tup == NULL)
				assert(false);
			while (--oparg >= 0) {
				PyObject *item = POP();
				PyTuple_SET_ITEM(tup, oparg, item);
			}
			PUSH(tup);
			DISPATCH();
		}
		case TARGET(BINARY_FLOOR_DIVIDE): {
			PyObject *divisor = POP();
			PyObject *dividend = TOP();
			PyObject *quotient = PyNumber_FloorDivide(dividend, divisor);
			Py_DECREF(dividend);
			Py_DECREF(divisor);
			SET_TOP(quotient);
			if (quotient == NULL)
				assert(false);
			DISPATCH();
		}
		case TARGET(BINARY_MULTIPLY): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *res = PyNumber_Multiply(left, right);
			Py_DECREF(left);
			Py_DECREF(right);
			SET_TOP(res);
			if (res == NULL)
				assert(false);
			DISPATCH();
		}
		case JUMP_FORWARD: {
			JUMPBY(oparg);
			DISPATCH();
		}
		case TARGET(BINARY_POWER): {
			PyObject *exp = POP();
			PyObject *base = TOP();
			PyObject *res = PyNumber_Power(base, exp, Py_None);
			Py_DECREF(base);
			Py_DECREF(exp);
			SET_TOP(res);
			if (res == NULL)
				assert(false);
			DISPATCH();
		}
		case TARGET(BINARY_SUBTRACT): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *diff = PyNumber_Subtract(left, right);
			Py_DECREF(right);
			Py_DECREF(left);
			SET_TOP(diff);
			if (diff == NULL)
				assert(false);
			DISPATCH();
		}
		case TARGET(BINARY_TRUE_DIVIDE): {
			PyObject *divisor = POP();
			PyObject *dividend = TOP();
			PyObject *quotient = PyNumber_TrueDivide(dividend, divisor);
			Py_DECREF(dividend);
			Py_DECREF(divisor);
			SET_TOP(quotient);
			if (quotient == NULL)
				assert(false);
			DISPATCH();
		}
    case TARGET(JUMP_IF_FALSE_OR_POP): {
      PyObject *cond = TOP();
      int err;
      if (Py_IsTrue(cond)) {
        STACK_SHRINK(1);
        Py_DECREF(cond);
        DISPATCH();
      }
      if (Py_IsFalse(cond)) {
        JUMPTO(oparg);
        DISPATCH();
      }
      err = PyObject_IsTrue(cond);
      if (err > 0) {
        STACK_SHRINK(1);
        Py_DECREF(cond);
      } else if (err == 0)
        JUMPTO(oparg);
      else
        goto error;
      DISPATCH();
    }
		case TARGET(POP_JUMP_IF_FALSE): {
			PyObject *cond = POP();
      int err;
			if (Py_IsTrue(cond)) {
				Py_DECREF(cond);
				DISPATCH();
			}
			if (Py_IsFalse(cond)) {
				Py_DECREF(cond);
				JUMPTO(oparg);
				DISPATCH();
			}
      err = PyObject_IsTrue(cond);
      Py_DECREF(cond);
      if (err > 0)
        ;
      else if (err == 0) {
        JUMPTO(oparg);
      } else {
        goto error;
      }
      DISPATCH();
		}
		case TARGET(POP_JUMP_IF_TRUE): {
			PyObject *cond = POP();
      int err;
			if (Py_IsFalse(cond)) {
				Py_DECREF(cond);
				DISPATCH();
			}
			if (Py_IsTrue(cond)) {
				Py_DECREF(cond);
				JUMPTO(oparg);
				DISPATCH();
			}
      err = PyObject_IsTrue(cond);
      Py_DECREF(cond);
      if (err > 0) {
        JUMPTO(oparg);
      } else if (err == 0) {
      } else {
        goto error;
      }
      DISPATCH();
		}
		case TARGET(COMPARE_OP): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *res = PyObject_RichCompare(left, right, oparg);
			SET_TOP(res);
			Py_DECREF(left);
			Py_DECREF(right);
			if (res == NULL)
				assert(false);
			DISPATCH();
		}
		case TARGET(BINARY_MODULO): {
			PyObject *divisor = POP();
			PyObject *dividend = TOP();
			PyObject *res;
			if (PyUnicode_CheckExact(dividend) && (
				!PyUnicode_Check(divisor) || PyUnicode_CheckExact(divisor))) {
				assert(false);
			} else {
				res = PyNumber_Remainder(dividend, divisor);
			}
			Py_DECREF(divisor);
			Py_DECREF(dividend);
			SET_TOP(res);
			if (res == NULL)
				assert(false);
			DISPATCH();
		}
		case TARGET(CALL_METHOD): {
			PyObject **sp, *res, *meth;

			sp = stack_pointer;

			meth = PEEK(oparg + 2);
			if (meth == NULL) {
        res = call_function(tstate, &sp, oparg, NULL);
        stack_pointer = sp;
        (void) POP(); // POP the NULL
			} else {
				res = call_function(tstate, &sp, oparg + 1, NULL);
				stack_pointer = sp;
			}

			PUSH(res);
			if (res == NULL) {
        goto error;
			}
			DISPATCH();
		}
		case TARGET(LOAD_ATTR): {
			PyObject *name = GETITEM(names, oparg);
			PyObject *owner = TOP();

			PyTypeObject *type = Py_TYPE(owner);
			PyObject *res;

			// Slow path
			res = PyObject_GetAttr(owner, name);
			Py_DECREF(owner);
			SET_TOP(res);
			if (res == NULL) {
        // printf("LOAD_ATTR fail for type %s, name %s\n", Py_TYPE(owner)->tp_name, (char *) PyUnicode_DATA(name));
        goto error;
			}
			DISPATCH();
		}
		case TARGET(JUMP_ABSOLUTE): {
			// printf("JUMP_ABSOLUTE oparg is %d\n", oparg);
			JUMPTO(oparg);
			DISPATCH();
		}
		case TARGET(LOAD_FAST): {
			PyObject *value = GETLOCAL(oparg);
			if (value == NULL) {
				printf("LOAD_FAST miss, oparg %d\n", oparg);
				assert(false);
			}
			Py_INCREF(value);
			PUSH(value);
			DISPATCH();
		}
		case TARGET(FOR_ITER): {
			// printf("FOR_ITER oparg is %d\n", oparg);
			PREDICTED(FOR_ITER);
			/* before: [iter]; after: [iter, iter()] *or* [] */
			PyObject *iter = TOP();
			PyObject *next = (*Py_TYPE(iter)->tp_iternext)(iter);
			if (next != NULL) {
				PUSH(next);
				PREDICT(STORE_FAST);
				// PREDICT(UNPACK_SEQUENCE);
				DISPATCH();
			}
			if (_PyErr_Occurred(tstate)) {
				assert(false);
			}
			// iterator ended normally
			STACK_SHRINK(1);
			Py_DECREF(iter);
			JUMPBY(oparg);
			DISPATCH();
		}
		case TARGET(GET_ITER): {
			// before: [obj]; after [getiter(obj)]
			PyObject *iterable = TOP();
			PyObject *iter = PyObject_GetIter(iterable);
			Py_DECREF(iterable);
			SET_TOP(iter);
			if (iter == NULL)
				assert(false);
			PREDICT(FOR_ITER);
			PREDICT(CALL_FUNCTION);
			DISPATCH();
		}
		case TARGET(LOAD_NAME): {
			PyObject *name = GETITEM(names, oparg);
			// printf("LOAD_NAME: name is %s\n", PyUnicode_1BYTE_DATA(name));
			PyObject *locals = f->f_locals;
			PyObject *v;
			if (locals == NULL) {
				assert(false);
			}
			if (PyDict_CheckExact(locals)) {
				v = PyDict_GetItemWithError(locals, name);
				if (v != NULL) {
					Py_INCREF(v);
				} else if (_PyErr_Occurred(tstate)) {
					assert(false);
				}
			} else {
				assert(false);
			}
			if (v == NULL) {
				v = PyDict_GetItemWithError(f->f_globals, name);
				if (v != NULL) {
					Py_INCREF(v);
				} else if (_PyErr_Occurred(tstate)) {
					assert(false);
				} else {
					if (PyDict_CheckExact(f->f_builtins)) {
						v = PyDict_GetItemWithError(f->f_builtins, name);
						if (v == NULL) {
			        printf("LOAD_NAME not found: name is %s\n", PyUnicode_1BYTE_DATA(name));
							assert(false);
						}
						Py_INCREF(v);
					} else {
						assert(false);
					}
				}
			}
			assert(v);
			PUSH(v);
			DISPATCH();
		}
		case TARGET(BINARY_ADD): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *sum;
			if (PyUnicode_CheckExact(left) && PyUnicode_CheckExact(right)) {
        sum = unicode_concatenate(tstate, left, right, f, next_instr);
			} else {
				sum = PyNumber_Add(left, right);
				Py_DECREF(left);
			}
			Py_DECREF(right);
			SET_TOP(sum);
			if (sum == NULL)
				assert(false);
			DISPATCH();
		}
		case TARGET(INPLACE_ADD): {
			PyObject *right = POP();
			PyObject *left = TOP();
			PyObject *sum;
			if (PyUnicode_CheckExact(left) && PyUnicode_CheckExact(right)) {
				assert(false);
			} else {
				sum = PyNumber_InPlaceAdd(left, right);
				Py_DECREF(left);
			}
			Py_DECREF(right);
			SET_TOP(sum);
			if (sum == NULL)
				assert(false);
			DISPATCH();
		}

		case TARGET(LOAD_GLOBAL): {
			PyObject *name;
			PyObject *v;
			if (PyDict_CheckExact(f->f_globals)
					&& PyDict_CheckExact(f->f_builtins)) {
				name = GETITEM(names, oparg);
				v = _PyDict_LoadGlobal((PyDictObject *) f->f_globals,
						(PyDictObject *) f->f_builtins,
						name);
				if (v == NULL) {
					printf("LOAD_GLOBAL miss symbol %s\n", (char *) PyUnicode_DATA(name));
					assert(false);
				}
				Py_INCREF(v);
			} else {
				assert(false);
			}
			PUSH(v);
			DISPATCH();
		}
		
		case TARGET(LOAD_CONST): {
			PREDICTED(LOAD_CONST);
			PyObject *value = GETITEM(consts, oparg);
			Py_INCREF(value);
			PUSH(value);
			DISPATCH();
		}

		case TARGET(CALL_FUNCTION): {
			PREDICTED(CALL_FUNCTION);
			PyObject **sp, *res;
			sp = stack_pointer;
			res = call_function(tstate, &sp, oparg, NULL);
			stack_pointer = sp;
			PUSH(res);
			if (res == NULL) {
				assert(false);
			}
			DISPATCH();
		}

		case TARGET(POP_TOP): {
			PyObject *value = POP();
			Py_DECREF(value);
			DISPATCH();
		}

		case TARGET(RETURN_VALUE): {
			retval = POP();
			// assert(f->f_iblock == 0);
			assert(EMPTY());
			f->f_state = FRAME_RETURNED;
			// goto exiting;
			goto exiting;
		}

		case TARGET(MAKE_FUNCTION): {
			PyObject *qualname = POP();
			PyObject *codeobj = POP();
			PyFunctionObject *func = (PyFunctionObject *)
					PyFunction_NewWithQualName(codeobj, f->f_globals, qualname);

			Py_DECREF(codeobj);
			Py_DECREF(qualname);
			if (func == NULL) {
				assert(false);
			}

			if (oparg & 0x08) {
        assert(PyTuple_CheckExact(TOP()));
        func->func_closure = POP();
			}
			if (oparg & 0x04) {
				assert(false);
			}
			if (oparg & 0x02) {
        assert(PyDict_CheckExact(TOP()));
        func->func_kwdefaults = POP();
			}
			if (oparg & 0x01) {
        assert(PyTuple_CheckExact(TOP()));
        func->func_defaults = POP();
			}

			PUSH((PyObject *) func);
			DISPATCH();
		}
		case TARGET(STORE_FAST): {
			PREDICTED(STORE_FAST);
			PyObject *value = POP();
			SETLOCAL(oparg, value);
			DISPATCH();
		}
		case TARGET(STORE_NAME): {
			PyObject *name = GETITEM(names, oparg);
			PyObject *v = POP();
			PyObject *ns = f->f_locals;
			int err;
			if (ns == NULL) {
				assert(false);
			}
			if (PyDict_CheckExact(ns)) {
				err = PyDict_SetItem(ns, name, v);
			} else {
				assert(false);
			}
			Py_DECREF(v);
			if (err != 0) {
				assert(false);
			}
			DISPATCH();
		}
		case TARGET(DELETE_FAST): {
			PyObject *v = GETLOCAL(oparg);
			if (v != NULL) {
				SETLOCAL(oparg, NULL);
				DISPATCH();
			}
			assert(false);
		}
    case TARGET(LOAD_CLOSURE): {
      PyObject *cell = freevars[oparg];
      Py_INCREF(cell);
      PUSH(cell);
      DISPATCH();
    }
    case TARGET(SETUP_WITH): {
      _Py_IDENTIFIER(__enter__);
      _Py_IDENTIFIER(__exit__);
      PyObject *mgr = TOP();
      PyObject *enter = special_lookup(tstate, mgr, &PyId___enter__);
      PyObject *res;
      if (enter == NULL) {
        goto error;
      }
      PyObject *exit = special_lookup(tstate, mgr, &PyId___exit__);
      if (exit == NULL) {
        Py_DECREF(enter);
        goto error;
      }
      SET_TOP(exit);
      Py_DECREF(mgr);
      res = _PyObject_CallNoArg(enter);
      Py_DECREF(enter);
      if (res == NULL)
        goto error;

      PyFrame_BlockSetup(f, SETUP_FINALLY, INSTR_OFFSET() + oparg,
          STACK_LEVEL());
      PUSH(res);
      DISPATCH();
    }
    case TARGET(POP_BLOCK): {
      PyFrame_BlockPop(f);
      DISPATCH();
    }
		default:
			printf("Can not handle opcode %d\n", opcode);
			assert(false);
		} /* switch */

		/* This should never be reached. Every opcode should end with DISPATCH()
			or goto error. */
		Py_UNREACHABLE();
error:
    if (!_PyErr_Occurred(tstate)) {
      fail(0);
    }

    PyTraceBack_Here(f);

    if (tstate->c_tracefunc != NULL) {
      fail(0);
    }
exception_unwind:
    f->f_state = FRAME_UNWINDING;
    while (f->f_iblock > 0) {
      PyTryBlock *b = &f->f_blockstack[--f->f_iblock];

      if (b->b_type == EXCEPT_HANDLER) {
        UNWIND_EXCEPT_HANDLER(b);
        continue;
      }
      UNWIND_BLOCK(b);
      if (b->b_type == SETUP_FINALLY) {
        PyObject *exc, *val, *tb;
        int handler = b->b_handler;
        _PyErr_StackItem *exc_info = tstate->exc_info;
        PyFrame_BlockSetup(f, EXCEPT_HANDLER, f->f_lasti, STACK_LEVEL());
        PUSH(exc_info->exc_traceback);
        PUSH(exc_info->exc_value);
        if (exc_info->exc_type != NULL) {
          PUSH(exc_info->exc_type);
        } else {
          Py_INCREF(Py_None);
          PUSH(Py_None);
        }
        _PyErr_Fetch(tstate, &exc, &val, &tb);
        _PyErr_NormalizeException(tstate, &exc, &val, &tb);
        if (tb != NULL) {
          PyException_SetTraceback(val, tb);
        } else {
          fail(0);
        }
        Py_INCREF(exc);
        exc_info->exc_type = exc;
        Py_INCREF(val);
        exc_info->exc_value = val;
        exc_info->exc_traceback = tb;
        if (tb == NULL)
          tb = Py_None;
        Py_INCREF(tb);
        PUSH(tb);
        PUSH(val);
        PUSH(exc);
        JUMPTO(handler);
        f->f_state = FRAME_EXECUTING;
        goto main_loop;
      }
    }

    break;
	} // main loop

  assert(retval == NULL);
  assert(_PyErr_Occurred(tstate));

  // Pop remaining stack entries
  while (!EMPTY()) {
    PyObject *o = POP();
    Py_XDECREF(o);
  }
  f->f_stackdepth = 0;
  f->f_state = FRAME_RAISED;

  printf("cur exception: %s\n", (char *) PyUnicode_DATA(PyObject_Str(tstate->curexc_value)));

exiting:
	/* pop frame */

exit_eval_frame:
	tstate->frame = f->f_back;
  
  if (getenv("DEBUG_EVAL")) {
    debug_eval_done_frame(f, retval);
  }
	return _Py_CheckFunctionResult(tstate, NULL, retval, __func__);
}

static PyObject *
make_coro(PyFrameConstructor *con, PyFrameObject *f) {
  PyObject *gen;
  int is_coro = ((PyCodeObject *)con->fc_code)->co_flags & CO_COROUTINE;

  Py_CLEAR(f->f_back);

  if (is_coro) {
    assert(false);
  } else if (((PyCodeObject *) con->fc_code)->co_flags & CO_ASYNC_GENERATOR) {
    assert(false);
  } else {
    gen = PyGen_NewWithQualName(f, con->fc_name, con->fc_qualname);
  }
  if (gen == NULL) {
    return NULL;
  }

  _PyObject_GC_TRACK(f);
  return gen;
}

PyObject *
_PyEval_Vector(PyThreadState *tstate, PyFrameConstructor *con,
		PyObject *locals,
		PyObject *const* args, size_t argcount,
		PyObject *kwnames) {
	PyFrameObject *f = _PyEval_MakeFrameVector(
		tstate, con, locals, args, argcount, kwnames);
	if (f == NULL) {
		return NULL;
	}

  if (((PyCodeObject *) con->fc_code)->co_flags & (CO_GENERATOR | CO_COROUTINE | CO_ASYNC_GENERATOR)) {
    return make_coro(con, f);
  }

	PyObject *retval = _PyEval_EvalFrame(tstate, f, 0);

	Py_DECREF(f);
	return retval;
}

// defined in cpy/Python/ceval.c
PyObject *
PyEval_EvalCode(PyObject *co, PyObject *globals, PyObject *locals) {
	PyThreadState *tstate = PyThreadState_GET();
	if (locals == NULL) {
		locals = globals;
	}

	PyObject *builtins = _PyEval_BuiltinsFromGlobals(tstate, globals); // borrowed ref
	PyDict_New(); // TODO follow cpy
	if (builtins == NULL) {
		return NULL;
	}

	PyFrameConstructor desc = {
		.fc_globals = globals,
		.fc_builtins = builtins,
		.fc_name = ((PyCodeObject *) co)->co_name,
		.fc_qualname = ((PyCodeObject *) co)->co_name,
		.fc_code = co,
		.fc_defaults = NULL,
		.fc_kwdefaults = NULL,
		.fc_closure = NULL,
	};
	return _PyEval_Vector(tstate, &desc, locals, NULL, 0, NULL);
}

PyObject *
_PyEval_GetBuiltins(PyThreadState *tstate) {
	PyFrameObject *frame = tstate->frame;
	if (frame != NULL) {
		return frame->f_builtins;
	}
	return tstate->interp->builtins;
}

PyObject *PyEval_GetGlobals(void) {
	PyThreadState *tstate = _PyThreadState_GET();
	PyFrameObject *current_frame = tstate->frame;
	if (current_frame == NULL) {
		return NULL;
	}
  assert(current_frame->f_globals != NULL);
  return current_frame->f_globals;
}

// return 0 on error, 1 on success
int _PyEval_SliceIndex(PyObject *v, Py_ssize_t *pi) {
	PyThreadState *tstate = _PyThreadState_GET();
	if (!Py_IsNone(v)) {
		Py_ssize_t x;
		if (_PyIndex_Check(v)) {
			x = PyNumber_AsSsize_t(v, NULL);
			if (x == -1 && _PyErr_Occurred(tstate))
				return 0;
		} else {
			assert(false);
		}
		*pi = x;
	}
	return 1;
}

PyObject *
PyEval_GetBuiltins(void) {
  PyThreadState *tstate = _PyThreadState_GET();
  return _PyEval_GetBuiltins(tstate);
}
