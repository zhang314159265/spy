#ifndef Py_ERRORS_H
#define Py_ERRORS_H

#include "object.h"
#include "sutil.h"

typedef struct _ts PyThreadState;

// Defined in cpy/Python/errors.c
PyObject *PyErr_NoMemory(void) {
	assert(false);
}

static inline PyObject *_PyErr_Occurred(PyThreadState *tstate);
static PyThreadState *_PyThreadState_GET(void);

PyObject *PyErr_Occurred(void) {
	// The caller must hold the GIL.
	// TODO add assert as cpy

	PyThreadState *tstate = _PyThreadState_GET();
	return _PyErr_Occurred(tstate);
}

#endif
