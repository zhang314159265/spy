#pragma once

#include "funcobject.h"
// #include "pystate.h"

enum _framestate {
	FRAME_CREATED = -2,
	FRAME_EXECUTING = 0,
	FRAME_RETURNED = 1,
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
	int f_lasti; // last instruction if called
	PyFrameState f_state;
	PyObject *f_localsplus[1]; // locals + stack
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

	Py_ssize_t extras = code->co_stacksize + code->co_nlocals;

	f = PyObject_GC_NewVar(PyFrameObject, &PyFrame_Type, extras);
	if (f == NULL) {
		return NULL;
	}

	extras = code->co_nlocals;
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

PyObject *
_PyEval_BuiltinsFromGlobals(PyThreadState *tstate, PyObject *globals)
{
	PyObject *builtins = _PyDict_GetItemIdWithError(globals, &PyId___builtins__);
	if (builtins) {
		assert(false);
	}
	if (PyErr_Occurred()) {
		return NULL;
	}

	return _PyEval_GetBuiltins(tstate);
}

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
