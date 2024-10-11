#pragma once

PyObject * _PyBuiltin_Init(PyInterpreterState *interp);
PyObject * PyModule_GetDict(PyObject *m);

static PyStatus
pycore_init_builtins(PyThreadState *tstate)
{
	PyInterpreterState *interp = tstate->interp;

	PyObject *bimod = _PyBuiltin_Init(interp);
	if (bimod == NULL) {
		assert(false);
	}

	PyObject *builtins_dict = PyModule_GetDict(bimod);
	if (builtins_dict == NULL) {
		assert(false);
	}
	Py_INCREF(builtins_dict);
	interp->builtins = builtins_dict;

	Py_DECREF(bimod);

	return _PyStatus_OK();
}
