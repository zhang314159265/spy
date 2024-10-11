#pragma once

#include "longobject.h"
#include "cpython/pystate.h"

#define _PY_NSMALLPOSINTS 257
#define _PY_NSMALLNEGINTS 5

struct _Py_unicode_ids {
	Py_ssize_t size;
	PyObject **array;
};

struct _Py_unicode_state {
	// for _Py_Identifier
	struct _Py_unicode_ids ids;
};

// The PyInterpreterState typedef is in Include/pystate.h
struct _is {
  PyLongObject *small_ints[_PY_NSMALLNEGINTS + _PY_NSMALLPOSINTS];
  struct pyruntimestate *runtime;

	_PyFrameEvalFunction eval_frame;

	struct _Py_unicode_state unicode;

	PyObject *builtins;

	// Dictionary of the sys module
	PyObject *sysdict;

	// sys.modules dictionary
	PyObject *modules;

	// importlib module
	PyObject *importlib;
};
