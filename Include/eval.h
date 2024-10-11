#pragma once

#include "frameobject.h"
#include "funcobject.h"

PyFrameObject *
_PyEval_MakeFrameVector(PyThreadState *tstate,
		PyFrameConstructor *con, PyObject *locals,
		PyObject *const *args, Py_ssize_t argcount,
		PyObject *kwnames) {
	PyCodeObject *co = (PyCodeObject*) con->fc_code;
	const Py_ssize_t total_args; // TODO follow cpy

	/* Create the frame */
	PyFrameObject *f = _PyFrame_New_NoTrack(tstate, con, locals);
	if (f == NULL) {
		return NULL;
	}
	PyObject **fastlocals = f->f_localsplus;
	PyObject **freevars = f->f_localsplus + co->co_nlocals;

	// Create a dictionary for keyword parameters (**kwargs)
	PyObject *kwdict = NULL;

	// Copy all positional arguments into local variables
	Py_ssize_t n = 0; // TODO follow cpy
	assert(n == 0);

	return f;
}

static inline PyObject *
_PyEval_EvalFrame(PyThreadState *tstate, PyFrameObject *f, int throwflag) {
	return tstate->interp->eval_frame(tstate, f, throwflag);
}

PyObject *
_PyEval_EvalFrameDefault(PyThreadState *tstate, PyFrameObject *f, int throwflag) {
	assert(false);
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

	PyObject *builtins = PyDict_New(); // TODO follow cpy
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
