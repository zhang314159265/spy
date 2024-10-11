#pragma once

#include "marshal.h"

#include "Copied/importlib.h"

struct _frozen {
	const char *name;
	const unsigned char *code;
	int size;
};

static PyObject *
import_get_module(PyThreadState *tstate, PyObject *name) {
	PyObject *modules = tstate->interp->modules;
	if (modules == NULL) {
		assert(false);
	}

	PyObject *m;
	Py_INCREF(modules);
	if (PyDict_CheckExact(modules)) {
		m = PyDict_GetItemWithError(modules, name);
		Py_XINCREF(m);
	} else {
		assert(false);
	}
	Py_DECREF(modules);
	return m;
}

static PyObject *
import_find_and_load(PyThreadState *tstate, PyObject *abs_name) {
	PyInterpreterState *interp = tstate->interp;

	// interp->importlib;
	assert(false);
}

PyObject *
PyImport_ImportModuleLevelObject(PyObject *name, PyObject *globals,
		PyObject *locals, PyObject *fromlist,
		int level) {
	PyThreadState *tstate = _PyThreadState_GET();
	PyObject *abs_name = NULL;
	PyObject *mod = NULL;

	printf("PyImport_ImportModuleLevelObject name is %s\n", PyUnicode_1BYTE_DATA(name));
	if (name == NULL) {
		assert(false);
	}

	if (PyUnicode_READY(name) < 0) {
		assert(false);
	}
	if (level < 0) {
		assert(false);
	}

	if (level > 0) {
		assert(false);
	} else {
		if (PyUnicode_GET_LENGTH(name) == 0) {
			assert(false);
		}
		abs_name = name;
		Py_INCREF(abs_name);
	}

	mod = import_get_module(tstate, abs_name);
	if (mod == NULL && _PyErr_Occurred(tstate)) {
		assert(false);
	}

	if (mod != NULL && mod != Py_None) {
		assert(false);
	} else {
		Py_XDECREF(mod);
		mod = import_find_and_load(tstate, abs_name);
		if (mod == NULL) {
			assert(false);
		}
	}
	assert(false);
}

PyObject *
PyImport_ImportModuleLevel(const char *name, PyObject *globals, PyObject *locals,
		PyObject *fromlist, int level)
{
	PyObject *nameobj, *mod;
	nameobj = PyUnicode_FromString(name);
	if (nameobj == NULL)
		return NULL;
	mod = PyImport_ImportModuleLevelObject(nameobj, globals, locals,
			fromlist, level);
	Py_DECREF(nameobj);
	return mod;
}

PyObject *
PyImport_Import(PyObject *module_name) {
	_Py_IDENTIFIER(__import__);
	_Py_IDENTIFIER(__builtin__);

	PyObject *globals = NULL;
	PyObject *builtins = NULL;

	PyThreadState *tstate = _PyThreadState_GET();

	PyObject *import_str = _PyUnicode_FromId(&PyId___import__);
	if (import_str == NULL) {
		return NULL;
	}

	// Get the builtins from current globals
	globals = PyEval_GetGlobals();
	if (globals != NULL) {
		assert(false);
	} else {
		builtins = PyImport_ImportModuleLevel("builtins",
				NULL, NULL, NULL, 0);
		if (builtins == NULL) {
			assert(false);
		}
		assert(false);
	}
	assert(false);
}

// defined in cpy/Python/import.c
PyObject *PyImport_ImportModule(const char *name) {
	PyObject *pname;
	PyObject *result;

	pname = PyUnicode_FromString(name);
	if (pname == NULL)
		return NULL;

	result = PyImport_Import(pname);
	Py_DECREF(pname);
	return result;
}

static const struct _frozen _PyImport_FrozenModules[] = {
	{"_frozen_importlib", _Py_M__importlib_bootstrap,
			(int)sizeof(_Py_M__importlib_bootstrap)},
	{0, 0, 0},
};

const struct _frozen *PyImport_FrozenModules = _PyImport_FrozenModules;

static const struct _frozen *
find_frozen(PyObject *name) {
	const struct _frozen *p;

	if (name == NULL)
		return NULL;
	
	for (p = PyImport_FrozenModules; ; p++) {
		if (p->name == NULL)
			return NULL;
		if (_PyUnicode_EqualToASCIIString(name, p->name))
			break;
	}
	return p;
}

int
PyImport_ImportFrozenModuleObject(PyObject *name) {
	const struct _frozen *p;
	PyObject *co;
	int ispackage;
	int size;

	p = find_frozen(name);

	if (p == NULL)
		return 0;

	if (p->code == NULL) {
		assert(false);
	}
	size = p->size;
	ispackage = (size < 0);
	if (ispackage)
		size = -size;
	co = PyMarshal_ReadObjectFromString((const char *) p->code, size);
	if (co == NULL)
		return -1;
	assert(false);
}

int
PyImport_ImportFrozenModule(const char *name) {
	PyObject *nameobj;
	int ret;

	nameobj = PyUnicode_InternFromString(name);
	if (nameobj == NULL)
		return -1;
	ret = PyImport_ImportFrozenModuleObject(nameobj);
	Py_DECREF(nameobj);
	return ret;
}
