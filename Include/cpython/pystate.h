#pragma once

#include "frameobject.h"

typedef struct _is PyInterpreterState;

typedef struct _err_stackitem {
  PyObject *exc_type, *exc_value, *exc_traceback;
  struct _err_stackitem *previous_item;
} _PyErr_StackItem;

// The PyThreadState typedef is in Include/pystate.h
struct _ts {
  PyInterpreterState *interp;
	PyFrameObject *frame;

	/* The exception currently being raised */
  PyObject *curexc_type;
  PyObject *curexc_value;
  PyObject *curexc_traceback;

  _PyErr_StackItem exc_state;
  _PyErr_StackItem *exc_info;
};

typedef PyObject *(*_PyFrameEvalFunction)(PyThreadState *tstate, PyFrameObject *, int);
