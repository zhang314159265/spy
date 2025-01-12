#pragma once

// #include "descrobject.h"

#define COMMON_FIELDS(PREFIX) \
	PyObject *PREFIX ## globals; \
	PyObject *PREFIX ## builtins; \
	PyObject *PREFIX ## name; \
	PyObject *PREFIX ## qualname; \
	PyObject *PREFIX ## code; /* A code object, the __code__ attribute */ \
	PyObject *PREFIX ## defaults; \
	PyObject *PREFIX ## kwdefaults; \
	PyObject *PREFIX ## closure;

#define PyFunction_AS_FRAME_CONSTRUCTOR(func) \
		((PyFrameConstructor *) &((PyFunctionObject *) (func))->func_globals)

#define PyFunction_Check(op) Py_IS_TYPE(op, &PyFunction_Type)

typedef struct {
	COMMON_FIELDS(fc_)
} PyFrameConstructor;

typedef struct {
	PyObject_HEAD
	COMMON_FIELDS(func_)

	PyObject *func_doc;
	PyObject *func_dict;

  PyObject *func_weakreflist;
	PyObject *func_module;

  PyObject *func_annotations;
	vectorcallfunc vectorcall;
} PyFunctionObject;

static void func_dealloc(PyFunctionObject *op);

PyObject *PyVectorcall_Call(PyObject *callable, PyObject *tuple, PyObject *kwargs);

static PyObject *func_get_name(PyFunctionObject *op, void *ignored);
static int func_set_name(PyFunctionObject *op, PyObject *value, void *ignore);

extern PyTypeObject PyFunction_Type;

PyObject *_PyEval_BuiltinsFromGlobals(PyThreadState *tstate, PyObject *globals);

PyObject *_PyFunction_Vectorcall(PyObject *func, PyObject *const *stack, size_t nargsf, PyObject *kwnames);

// defined in cpy/Objects/funcobject.c
PyObject *PyFunction_NewWithQualName(PyObject *code, PyObject *globals, PyObject *qualname) {
	assert(globals != NULL);
	assert(PyDict_Check(globals));
	Py_INCREF(globals);

	PyThreadState *tstate = _PyThreadState_GET();

	PyCodeObject *code_obj = (PyCodeObject *) code;
	Py_INCREF(code_obj);

	PyObject *name = code_obj->co_name;
	assert(name != NULL);
	Py_INCREF(name);
	if (!qualname) {
		qualname = name;
	}
	Py_INCREF(qualname);

	PyObject *consts = code_obj->co_consts;
	assert(PyTuple_Check(consts));
	PyObject *doc;
	if (PyTuple_Size(consts) >= 1) {
		doc = PyTuple_GetItem(consts, 0);
		if (!PyUnicode_Check(doc)) {
			doc = Py_None;
		}
	} else {
		doc = Py_None;
	}
	Py_INCREF(doc);

	_Py_IDENTIFIER(__name__);
	PyObject *module = _PyDict_GetItemIdWithError(globals, &PyId___name__);
	PyObject *builtins = NULL;
	if (module == NULL && _PyErr_Occurred(tstate)) {
		assert(false);
	}
	Py_XINCREF(module);

	builtins = _PyEval_BuiltinsFromGlobals(tstate, globals);

	if (builtins == NULL) {
		assert(false);
	}
	Py_INCREF(builtins);

	PyFunctionObject *op = PyObject_GC_New(PyFunctionObject, &PyFunction_Type);
	if (op == NULL) {
		assert(false);
	}

	op->func_globals = globals;
	op->func_builtins = builtins;
	op->func_name = name;
	op->func_qualname = qualname;

	op->func_code = (PyObject *) code_obj;
	op->func_defaults = NULL;
	op->func_kwdefaults = NULL;
	op->func_closure = NULL;
	op->func_doc = doc;
	op->func_dict = NULL;
  op->func_weakreflist = NULL;
	op->func_module = module;
  op->func_annotations = NULL;
	op->vectorcall = _PyFunction_Vectorcall;

	return (PyObject *) op;
}

static int
func_clear(PyFunctionObject *op) {
  Py_CLEAR(op->func_code);
  Py_CLEAR(op->func_globals);
  Py_CLEAR(op->func_builtins);
  Py_CLEAR(op->func_name);
  Py_CLEAR(op->func_qualname);
  Py_CLEAR(op->func_module);
  Py_CLEAR(op->func_defaults);
  Py_CLEAR(op->func_kwdefaults);
  Py_CLEAR(op->func_doc);
  Py_CLEAR(op->func_dict);
  Py_CLEAR(op->func_closure);
  // Py_CLEAR(op->func_annotations);
  return 0;
}

static void func_dealloc(PyFunctionObject *op) {
  _PyObject_GC_UNTRACK(op);
  if (op->func_weakreflist != NULL) {
    assert(false);
  }
  (void) func_clear(op);
  PyObject_GC_Del(op);
}

static PyObject *func_get_name(PyFunctionObject *op, void *ignored) {
  Py_INCREF(op->func_name);
  return op->func_name;
}

static int func_set_name(PyFunctionObject *op, PyObject *value, void *ignore) {
  if (value == NULL || !PyUnicode_Check(value)) {
    assert(false);
  }
  Py_INCREF(value);
  Py_XSETREF(op->func_name, value);
  return 0;
}
