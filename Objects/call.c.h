#pragma once

PyObject *
_Py_CheckFunctionResult(PyThreadState *tstate, PyObject *callable,
		PyObject *result, const char *where) {
	assert((callable != NULL) ^ (where != NULL));

	if (result == NULL) {
		assert(false);
	} else {
		if (_PyErr_Occurred(tstate)) {
			assert(false);
		}
	}
	return result;
}

PyObject *_PyFunction_Vectorcall(PyObject *func, PyObject *const *stack, size_t nargsf, PyObject *kwnames) {
	assert(PyFunction_Check(func));
	PyFrameConstructor *f = PyFunction_AS_FRAME_CONSTRUCTOR(func);
	Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
	// printf("nargs %ld\n", nargs);
	assert(nargs >= 0);
	PyThreadState *tstate = _PyThreadState_GET();
	assert(nargs == 0 || stack != NULL);
	if (((PyCodeObject *)f->fc_code)->co_flags & CO_OPTIMIZED) {
		return _PyEval_Vector(tstate, f, NULL, stack, nargs, kwnames);
	} else {
		assert(false);
	}
}


PyObject *PyVectorcall_Call(PyObject *callable, PyObject *tuple, PyObject *kwargs) {
	assert(false);
}
