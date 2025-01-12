#pragma once

PyObject * _PyBuiltin_Init(PyInterpreterState *interp);
PyObject * PyModule_GetDict(PyObject *m);
PyObject *_PyDict_GetItemStringWithError(PyObject *, const char *);
PyStatus _PyBuiltins_AddExceptions(PyObject *bltinmod);

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

  PyStatus status = _PyBuiltins_AddExceptions(bimod);
  if (_PyStatus_EXCEPTION(status)) {
    return status;
  }

	Py_DECREF(bimod);

  PyObject *import_func = _PyDict_GetItemStringWithError(interp->builtins,
      "__import__");
  if (import_func == NULL) {
    assert(false);
  }
  interp->import_func = Py_NewRef(import_func);

  assert(!_PyErr_Occurred(tstate));
	return _PyStatus_OK();
}
