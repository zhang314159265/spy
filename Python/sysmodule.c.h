#pragma once

static PyObject *
sys_get_object_id(PyThreadState *tstate, _Py_Identifier *key) {
	PyObject *sd = tstate->interp->sysdict;
	if (sd == NULL) {
		return NULL;
	}
	assert(false);
}

PyObject *_PySys_GetObjectId(_Py_Identifier *key) {
	PyThreadState *tstate = _PyThreadState_GET();
	return sys_get_object_id(tstate, key);
}

static struct PyModuleDef sysmodule = {
	PyModuleDef_HEAD_INIT,
	.m_name = "sys",
};

PyStatus
_PySys_Create(PyThreadState *tstate, PyObject **sysmod_p) {
	PyInterpreterState *interp = tstate->interp;

	PyObject *modules = PyDict_New();
	if (modules == NULL) {
		assert(false);
	}
	interp->modules = modules;

	PyObject *sysmod = _PyModule_CreateInitialized(&sysmodule, PYTHON_API_VERSION);
	if (sysmod == NULL) {
		assert(false);
	}

	PyObject *sysdict = PyModule_GetDict(sysmod);
	if (sysdict == NULL) {
		assert(false);
	}
	Py_INCREF(sysdict);
	interp->sysdict = sysdict;

	if (PyDict_SetItemString(sysdict, "modules", interp->modules) < 0) {
		assert(false);
	}

	*sysmod_p = sysmod;
	return _PyStatus_OK();
}
