#pragma once

#include "structseq.h"
#include "namespaceobject.h"

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

static PyObject *
list_builtin_module_names(void) {
  PyObject *list = PyList_New(0);
  if (list == NULL) {
    return NULL;
  }
  for (Py_ssize_t i = 0; PyImport_Inittab[i].name != NULL; i++) {
    PyObject *name = PyUnicode_FromString(PyImport_Inittab[i].name);
    if (name == NULL) {
      fail(0);
    }
    if (PyList_Append(list, name) < 0) {
      Py_DECREF(name);
      fail(0);
    }
    Py_DECREF(name);
  }
  if (PyList_Sort(list) != 0) {
    fail(0);
  }
  PyObject *tuple = PyList_AsTuple(list);
  Py_DECREF(list);
  return tuple;
}

static PyTypeObject FlagsType;

static int
set_flags_from_config(PyInterpreterState *interp, PyObject *flags) {
  Py_ssize_t pos = 0;

#define SetFlagObj(expr) \
  do { \
    PyObject *value = (expr); \
    if (value == NULL) { \
      return -1; \
    } \
    Py_XDECREF(PyStructSequence_GET_ITEM(flags, pos)); \
    PyStructSequence_SET_ITEM(flags, pos, value); \
    pos++; \
  } while (0)
#define SetFlag(expr) SetFlagObj(PyLong_FromLong(expr))

  // order matters
  SetFlag(0); // SetFlag(config->optimization_level);
  SetFlag(0); // SetFlag(config->verbose);
  SetFlag(0); // SetFlag(!config->use_environment);
  return 0;
}

static PyObject *
make_flags(PyInterpreterState *interp) {
  PyObject *flags = PyStructSequence_New(&FlagsType);
  if (flags == NULL) {
    return NULL;
  }
  if (set_flags_from_config(interp, flags) < 0) {
    Py_DECREF(flags);
    return NULL;
  }
  return flags;
}

#define SET_SYS(key, value) \
  do { \
    PyObject *v = (value); \
    if (v == NULL) { \
      fail(0); \
    } \
    res = PyDict_SetItemString(sysdict, key, v); \
    Py_DECREF(v); \
    if (res < 0) { \
      fail(0); \
    } \
  } while (0)

#define SET_SYS_FROM_STRING(key, value) \
  SET_SYS(key, PyUnicode_FromString(value))

static PyStructSequence_Field flags_fields[] = {
  {"optimize", "-O or -OO"},
  {"verbose", "-v"},
  {"ignore_environment", "-E"},
  {0},
};

struct PyStructSequence_Desc flags_desc = {
  .name = "sys.flags",
  .doc = "",
  .fields = flags_fields,
  .n_in_sequence = 17,
};

const char *Py_GetPlatform(void);

static PyObject *
make_version_info(PyThreadState *tstate) {
  // fail(0);
  // TODO follow cpy
  return NULL;
}

const char *_PySys_ImplCacheTag = "cpytion-310"; // TODO follow cpy

static PyObject *
make_impl_info(PyObject *version_info) {
  int res;
  PyObject *impl_info, *value, *ns;

  impl_info = PyDict_New();
  if (impl_info == NULL)
    return NULL;

  value = PyUnicode_FromString(_PySys_ImplCacheTag);
  if (value == NULL)
    fail(0);
  res = PyDict_SetItemString(impl_info, "cache_tag", value);
  Py_DECREF(value);
  if (res < 0)
    fail(0);

  ns = _PyNamespace_New(impl_info);
  Py_DECREF(impl_info);
  return ns;
}

static PyStatus
_PySys_InitCore(PyThreadState *tstate, PyObject *sysdict) {
  PyObject *version_info;
  int res;

  SET_SYS("builtin_module_names", list_builtin_module_names());
  SET_SYS_FROM_STRING("platform", Py_GetPlatform());

  if (FlagsType.tp_name == 0) {
    if (_PyStructSequence_InitType(&FlagsType, &flags_desc,
        Py_TPFLAGS_DISALLOW_INSTANTIATION) < 0) {
      fail(0);
    }
  }
  SET_SYS("flags", make_flags(tstate->interp));

  SET_SYS("meta_path", PyList_New(0));
  SET_SYS("path_hooks", PyList_New(0));
  SET_SYS("path_importer_cache", PyDict_New());

  version_info = make_version_info(tstate);
  SET_SYS("implementation", make_impl_info(version_info));
  return _PyStatus_OK();
}

int _PyImport_FixupBuiltin(PyObject *mod, const char *name, PyObject *modules);

PyStatus
_PySys_Create(PyThreadState *tstate, PyObject **sysmod_p) {
  PyStatus status;
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

  status = _PySys_InitCore(tstate, sysdict);
  if (_PyStatus_EXCEPTION(status)) {
    return status;
  }

  if (_PyImport_FixupBuiltin(sysmod, "sys", interp->modules) < 0) {
    fail(0);
  }

  assert(!_PyErr_Occurred(tstate));

	*sysmod_p = sysmod;
	return _PyStatus_OK();
}

int _PySys_UpdateConfig(PyThreadState *tstate) {
  PyInterpreterState *interp = tstate->interp;
  PyObject *sysdict = interp->sysdict;
  int res;

  {
    // TODO follow cpy to setup sys.path
    PyObject *pylist = PyList_New(1);
    PyList_SET_ITEM(pylist, 0, PyUnicode_FromString(""));
    SET_SYS("path", pylist);
  }

  SET_SYS("dont_write_bytecode", PyBool_FromLong(true)); // TODO follow cpy

  PyDict_SetItemString(sysdict, "pycache_prefix", Py_None);
  return 0;
}
