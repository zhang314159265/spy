#pragma once

#include "frameobject.h"

typedef struct _is PyInterpreterState;

// The PyThreadState typedef is in Include/pystate.h
struct _ts {
  PyInterpreterState *interp;
	PyFrameObject *frame;

	/* The exception currently being raised */
	PyObject *curexc_type;
};

typedef PyObject *(*_PyFrameEvalFunction)(PyThreadState *tstate, PyFrameObject *, int);
