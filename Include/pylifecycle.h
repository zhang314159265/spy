#pragma once

#include "import.h"

// cpy does not have this header file. Put functions in cpy/Python/pylifecycle.c here

PyStatus _PyExc_Init(PyInterpreterState *interp);

static PyStatus
pycore_init_types(PyInterpreterState *interp) {
	PyStatus status;
	int is_main_interp = _Py_IsMainInterpreter(interp);
	if (is_main_interp) {
		status = _PyTypes_Init();
		if (_PyStatus_EXCEPTION(status)) {
			return status;
		}
	}

  status = _PyExc_Init(interp);
  if (_PyStatus_EXCEPTION(status)) {
    return status;
  }
	return _PyStatus_OK();
}

static PyStatus
init_sys_streams(PyThreadState *tstate) {
	PyObject *iomod = NULL;

	assert(false); // not implemented yet since it (indirectly) relies on frozenmodule

	if(!(iomod = PyImport_ImportModule("io"))) {
		assert(false);
	}
	assert(false);
}

static PyStatus
init_interp_main(PyThreadState *tstate) {
	PyStatus status;

	#if 0 // init sys indirectly relies on frozenmodule
	status = init_sys_streams(tstate);
	if(_PyStatus_EXCEPTION(status)) {
		return status;
	}
	#endif

	return _PyStatus_OK();
}

static PyStatus
pyinit_main(PyThreadState *tstate) {
	PyInterpreterState *interp = tstate->interp;

	PyStatus status = init_interp_main(tstate);
	if (_PyStatus_EXCEPTION(status)) {
		return status;
	}
	return _PyStatus_OK();
}

static int
init_importlib(PyThreadState *tstate, PyObject *sysmod) {
	return 0; // TODO init_importlib not implemented yet since it replies on frozenmodules
	assert(!_PyErr_Occurred(tstate));

	if (PyImport_ImportFrozenModule("_frozen_importlib") <= 0) {
		return -1;
	}

	assert(false);
}
