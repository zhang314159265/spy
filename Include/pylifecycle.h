#pragma once

#include "import.h"
#include "call.h"

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
init_importlib_external(PyThreadState *tstate) {
  PyObject *value;
  value = PyObject_CallMethod(tstate->interp->importlib,
      "_install_external_importers", "");
  if (value == NULL) {
    fail(0);
  }
  Py_DECREF(value);
  return _PyImportZip_Init(tstate);
}

int _PySys_UpdateConfig(PyThreadState *tstate);

static int
interpreter_update_config(PyThreadState *tstate, int only_update_path_config) {
  if (_PySys_UpdateConfig(tstate) < 0) {
    return -1;
  }
  return 0;
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

  if (interpreter_update_config(tstate, 1) < 0) {
    fail(0);
  }

  status = init_importlib_external(tstate);
  if (_PyStatus_EXCEPTION(status)) {
    return status;
  }

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
	assert(!_PyErr_Occurred(tstate));

  PyInterpreterState *interp = tstate->interp;

	if (PyImport_ImportFrozenModule("_frozen_importlib") <= 0) {
		return -1;
	}
  PyObject *importlib = PyImport_AddModule("_frozen_importlib");
  if (importlib == NULL) {
    return -1;
  }
  interp->importlib = Py_NewRef(importlib);

  PyObject *imp_mod = _PyImport_BootstrapImp(tstate);
  if (imp_mod == NULL) {
    return -1;
  }

  if (_PyImport_SetModuleString("_imp", imp_mod) < 0) {
    fail(0);
  }

  PyObject *value = PyObject_CallMethod(importlib, "_install",
      "OO", sysmod, imp_mod);

  Py_DECREF(imp_mod);
  if (value == NULL) {
    return -1;
  }
  Py_DECREF(value);

  assert(!_PyErr_Occurred(tstate));
  return 0;
}

PyStatus
_Py_PreInitializeFromConfig(const PyConfig *config,
    const /* _PyArgv */ void *args) {
  // TODO not implemented yet
  return _PyStatus_OK();
}
