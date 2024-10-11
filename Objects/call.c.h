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
