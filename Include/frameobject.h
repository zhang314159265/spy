#pragma once

#include "funcobject.h"
// #include "pystate.h"

enum _framestate {
	FRAME_CREATED = -2,
};

typedef signed char PyFrameState;

struct _frame {
	PyObject_VAR_HEAD
	struct _frame *f_back;
	PyCodeObject *f_code;
	PyObject *f_builtins;
	PyObject *f_globals;
	PyObject *f_locals;
	PyObject **f_valuestack; // points after the last local
	PyFrameState f_state;
	PyObject *f_localsplus[1]; // locals + stack
};

typedef struct _frame PyFrameObject;

PyTypeObject PyFrame_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "frame",
	.tp_basicsize = sizeof(PyFrameObject),
	.tp_itemsize = sizeof(PyObject *),
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


