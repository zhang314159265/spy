#pragma once

#include "funcobject.h"
// #include "pystate.h"

typedef struct {
  int b_type;
  int b_handler;
  int b_level;
} PyTryBlock;

enum _framestate {
	FRAME_CREATED = -2,
  FRAME_SUSPENDED = -1,
	FRAME_EXECUTING = 0,
	FRAME_RETURNED = 1,
  FRAME_UNWINDING = 2,
  FRAME_RAISED = 3,
};

typedef signed char PyFrameState;

// defined in cpy/Include/cpython/frameobject.h
struct _frame {
	PyObject_VAR_HEAD
	struct _frame *f_back;
	PyCodeObject *f_code;
	PyObject *f_builtins;
	PyObject *f_globals;
	PyObject *f_locals;
	PyObject **f_valuestack; // points after the last local
	int f_stackdepth;

  PyObject *f_gen;

	int f_lasti; // last instruction if called
	PyFrameState f_state;
  int f_iblock;
  PyTryBlock f_blockstack[CO_MAXBLOCKS];
	PyObject *f_localsplus[1]; // locals + stack, must be the very last field
};

typedef struct _frame PyFrameObject;

static void frame_dealloc(PyFrameObject *f);

PyTypeObject PyFrame_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "frame",
	.tp_basicsize = sizeof(PyFrameObject),
	.tp_itemsize = sizeof(PyObject *),
	.tp_dealloc = (destructor) frame_dealloc,
};

static inline PyFrameObject *
frame_alloc(PyCodeObject *code) {
	PyFrameObject *f = NULL;

  Py_ssize_t ncells = PyTuple_GET_SIZE(code->co_cellvars);
  Py_ssize_t nfrees = PyTuple_GET_SIZE(code->co_freevars);
	Py_ssize_t extras = code->co_stacksize + code->co_nlocals + ncells + nfrees;

	f = PyObject_GC_NewVar(PyFrameObject, &PyFrame_Type, extras);
	if (f == NULL) {
		return NULL;
	}

	extras = code->co_nlocals + ncells + nfrees;
	f->f_valuestack = f->f_localsplus + extras;
	for (Py_ssize_t i = 0; i < extras; i++) {
		f->f_localsplus[i] = NULL;
	}
	return f;
}

typedef struct _ts PyThreadState;

PyFrameObject *
_PyFrame_New_NoTrack(PyThreadState *tstate, PyFrameConstructor *con, PyObject *locals);

_Py_IDENTIFIER(__builtins__);

PyObject * _PyEval_GetBuiltins(PyThreadState *tstate);

PyObject *_PyEval_BuiltinsFromGlobals(PyThreadState *tstate, PyObject *globals);

static void frame_dealloc(PyFrameObject *f) {
	PyObject **valuestack = f->f_valuestack;
	for (PyObject **p = f->f_localsplus; p < valuestack; p++) {
		Py_CLEAR(*p);
	}

	// Free stack
	for (int i = 0; i < f->f_stackdepth; i++) {
		assert(false);
	}
	f->f_stackdepth = 0;

	Py_XDECREF(f->f_back);
	Py_DECREF(f->f_builtins);
	Py_DECREF(f->f_globals);
	Py_CLEAR(f->f_locals);

	PyCodeObject *co = f->f_code;
	PyObject_GC_Del(f);
	Py_DECREF(co);
}

static inline int _PyFrame_IsRunnable(struct _frame *f) {
  return f->f_state < FRAME_EXECUTING;
}

static inline int _PyFrame_IsExecuting(struct _frame *f) {
  return f->f_state == FRAME_EXECUTING;
}

static inline int _PyFrameHasCompleted(struct _frame *f) {
  return f->f_state > FRAME_EXECUTING;
}

void PyFrame_BlockSetup(PyFrameObject *f, int type, int handler, int level);
PyTryBlock *PyFrame_BlockPop(PyFrameObject *f);

int PyFrame_GetLineNumber(PyFrameObject *f) {
  // TODO follow cpy
  return 0;
}

#define PyFrame_Check(op) Py_IS_TYPE(op, &PyFrame_Type)
