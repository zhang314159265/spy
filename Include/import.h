#pragma once

#include "marshal.h"

#include "Copied/importlib.h"
#include "Copied/importlib_external.h"
#include "moduleobject.h"
#include "weakrefobject.h"

_Py_IDENTIFIER(__path__);

struct _inittab {
  const char *name;
  PyObject *(*initfunc)(void);
};

extern struct _inittab _PyImport_Inittab[];

struct _inittab *PyImport_Inittab = _PyImport_Inittab;

static PyObject *extensions = NULL;

PyObject *PyModule_NewObject(PyObject *name);

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

PyObject *_PyObject_CallMethodIdObjArgs(PyObject *obj, struct _Py_Identifier *name, ...);

static PyObject *
import_find_and_load(PyThreadState *tstate, PyObject *abs_name) {
  _Py_IDENTIFIER(_find_and_load);
  PyObject *mod = NULL;
	PyInterpreterState *interp = tstate->interp;

  mod = _PyObject_CallMethodIdObjArgs(interp->importlib,
      &PyId__find_and_load, abs_name,
      interp->import_func, NULL);

  return mod;
}

PyObject *_PyObject_GetAttrId(PyObject *v, _Py_Identifier *name);

static int
import_ensure_initialized(PyInterpreterState *interp, PyObject *mod, PyObject *name) {
  PyObject *spec;
  _Py_IDENTIFIER(__spec__);

  spec = _PyObject_GetAttrId(mod, &PyId___spec__);
  int busy = _PyModuleSpec_IsInitializing(spec);
  Py_XDECREF(spec);
  if (busy) {
    fail(0);
  }
  return 0;
}

PyObject *PyImport_ImportModuleLevelObject(PyObject *name, PyObject *globals,
		PyObject *locals, PyObject *fromlist,
		int level) {
	PyThreadState *tstate = _PyThreadState_GET();
  _Py_IDENTIFIER(_handle_fromlist);
	PyObject *abs_name = NULL;
  PyObject *final_mod = NULL;
	PyObject *mod = NULL;
  PyObject *package = NULL;
  PyInterpreterState *interp = tstate->interp;
  int has_from;

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
    if (import_ensure_initialized(tstate->interp, mod, abs_name) < 0) {
      fail(0);
    }
	} else {
		Py_XDECREF(mod);
		mod = import_find_and_load(tstate, abs_name);
		if (mod == NULL) {
			assert(false);
		}
	}

  has_from = 0;
  if (fromlist != NULL && fromlist != Py_None) {
    has_from = PyObject_IsTrue(fromlist);
    if (has_from < 0) 
      fail(0);
  }
  if (!has_from) {
    Py_ssize_t len = PyUnicode_GET_LENGTH(name);
    if (level == 0 || len > 0) {
      Py_ssize_t dot;

      dot = PyUnicode_FindChar(name, '.', 0, len, 1);
      if (dot == -2) {
        fail(0);
      }
      if (dot == -1) {
        // No dot in module name, simple xit
        final_mod = mod;
        Py_INCREF(mod);
        goto error;
      }
      fail(0);
    } else {
      final_mod = mod;
      Py_INCREF(mod);
    }
  } else {
    PyObject *path;
    if (_PyObject_LookupAttrId(mod, &PyId___path__, &path) < 0) {
      fail(0);
    }
    if (path) {
      Py_DECREF(path);
      final_mod = _PyObject_CallMethodIdObjArgs(
        interp->importlib, &PyId__handle_fromlist,
        mod, fromlist, interp->import_func, NULL);
    } else {
      final_mod = mod;
      Py_INCREF(mod);
    }
  }

 error:
  Py_XDECREF(abs_name);
  Py_XDECREF(mod);
  Py_XDECREF(package);
  if (final_mod == NULL) {
    fail(0);
  }
  return final_mod;
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

PyObject *PyImport_Import(PyObject *module_name);

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
  {"_frozen_importlib_external", _Py_M__importlib_bootstrap_external,
      (int)sizeof(_Py_M__importlib_bootstrap_external)},
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

static PyObject *
import_add_module(PyThreadState *tstate, PyObject *name) {
  PyObject *modules = tstate->interp->modules;
  if (modules == NULL) {
    assert(false);
  }
  PyObject *m;
  if (PyDict_CheckExact(modules)) {
    m = PyDict_GetItemWithError(modules, name);
    Py_XINCREF(m);
  } else {
    assert(false);
  }
  if (_PyErr_Occurred(tstate)) {
    return NULL;
  }
  if (m != NULL && PyModule_Check(m)) {
    return m;
  }
  Py_XDECREF(m);
  m = PyModule_NewObject(name);
  if (m == NULL)
    return NULL;
  if (PyObject_SetItem(modules, name, m) != 0) {
    Py_DECREF(m);
    return NULL;
  }

  return m;
}

PyObject *PyModule_GetDict(PyObject *m);

static PyObject *
module_dict_for_exec(PyThreadState *tstate, PyObject *name) {
  _Py_IDENTIFIER(__builtins__);
  PyObject *m, *d;

  m = import_add_module(tstate, name);
  if (m == NULL)
    return NULL;
  d = PyModule_GetDict(m);
  int r = _PyDict_ContainsId(d, &PyId___builtins__);
  if (r == 0) {
    r = _PyDict_SetItemId(d, &PyId___builtins__,
        PyEval_GetBuiltins());
  }
  if (r < 0) {
    assert(false);
  }

  Py_INCREF(d);
  Py_DECREF(m);
  return d;
}

static PyObject *
exec_code_in_module(PyThreadState *tstate, PyObject *name,
    PyObject *module_dict, PyObject *code_object) {
  PyObject *v, *m;

  v = PyEval_EvalCode(code_object, module_dict, module_dict);
  if (v == NULL) {
    assert(false);
  }
  Py_DECREF(v);

  m = import_get_module(tstate, name);
  if (m == NULL) {
    assert(false);
  }

  return m;
}

int
PyImport_ImportFrozenModuleObject(PyObject *name) {
  PyThreadState *tstate = _PyThreadState_GET();
	const struct _frozen *p;
	PyObject *co, *m, *d;
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
  if (!PyCode_Check(co)) {
    assert(false);
  }
  if (ispackage) {
    assert(false);
  }

  d = module_dict_for_exec(tstate, name);
  if (d == NULL) {
    assert(false);
  }
  m = exec_code_in_module(tstate, name, d, co);
  Py_DECREF(d);
  if (m == NULL) {
    assert(false);
  }

  Py_DECREF(co);
  Py_DECREF(m);
  return 1;
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

PyObject *
PyImport_AddModuleObject(PyObject *name) {
  PyThreadState *tstate = _PyThreadState_GET();
  PyObject *mod = import_add_module(tstate, name);
  if (mod) {
    PyObject *ref = PyWeakref_NewRef(mod, NULL);
    Py_DECREF(mod);
    if (ref == NULL) {
      return NULL;
    }
    mod = PyWeakref_GetObject(ref);
    Py_DECREF(ref);
  }
  return mod;
}

PyObject *
PyImport_AddModule(const char *name) {
  PyObject *nameobj = PyUnicode_FromString(name);
  if (nameobj == NULL) {
    return NULL;
  }
  PyObject *module = PyImport_AddModuleObject(nameobj);
  Py_DECREF(nameobj);
  return module;
}

static int
exec_builtin_or_dynamic(PyObject *mod) {
  PyModuleDef *def;
  void *state;

  if (!PyModule_Check(mod)) {
    return 0;
  }

  def = PyModule_GetDef(mod);
  if (def == NULL) {
    return 0;
  }

  state = PyModule_GetState(mod);
  if (state) {
    return 0;
  }

  return PyModule_ExecDef(mod, def);
}

static PyObject *
import_find_extension(PyThreadState *tstate, PyObject *name,
    PyObject *filename) {
  if (extensions == NULL) {
    return NULL;
  }
  PyObject *key = PyTuple_Pack(2, filename, name);
  if (key == NULL) {
    return NULL;
  }
  PyModuleDef* def = (PyModuleDef *) PyDict_GetItemWithError(extensions, key);
  Py_DECREF(key);
  if (def == NULL) {
    return NULL;
  }

  if (def->m_size == -1) {
    fail(0);
  } else {
    fail(0);
  }
  fail(0);
}

static PyObject *
create_builtin(PyThreadState *tstate, PyObject *name, PyObject *spec);
PyObject *_PyImport_BootstrapImp(PyThreadState *tstate);

static PyObject *
is_frozen_package(PyObject *name) {
  const struct _frozen *p = find_frozen(name);
  int size;

  if (p == NULL) {
    fail(0);
  }
  size = p->size;

  if (size < 0)
    Py_RETURN_TRUE;
  else
    Py_RETURN_FALSE;
}

static PyObject *
_imp_is_frozen_package_impl(PyObject *module, PyObject *name) {
  return is_frozen_package(name);
}

static PyObject *
_imp_is_frozen_package(PyObject *module, PyObject *arg) {
  PyObject *return_value = NULL;
  PyObject *name;

  if (!PyUnicode_Check(arg)) {
    fail(0);
  }
  if (PyUnicode_READY(arg) == -1) {
    fail(0);
  }
  name = arg;
  return_value = _imp_is_frozen_package_impl(module, name);
exit:
  return return_value;
}

#define _IMP_IS_FROZEN_METHODDEF \
  {"is_frozen", (PyCFunction) _imp_is_frozen, METH_O, ""},

#define _IMP_IS_FROZEN_PACKAGE_METHODDEF \
  {"is_frozen_package", (PyCFunction) _imp_is_frozen_package, METH_O, ""},

#define _IMP_IS_BUILTIN_METHODDEF \
  {"is_builtin", (PyCFunction) _imp_is_builtin, METH_O, ""},

static PyObject *_imp_is_frozen(PyObject *module, PyObject *arg);
static PyObject *_imp_is_builtin(PyObject *module, PyObject *arg);
static PyObject *_imp_create_builtin(PyObject *module, PyObject *spec);
static PyObject *_imp_exec_builtin(PyObject *module, PyObject *mod);

void _PyImport_AcquireLock(void) {
  // TODO follow cpy
  return;
}

static PyObject *
_imp_acquire_lock_impl(PyObject *module) {
  _PyImport_AcquireLock();
  Py_RETURN_NONE;
}

static PyObject *
_imp_acquire_lock(PyObject *module, PyObject *ignored) {
  return _imp_acquire_lock_impl(module);
}

static PyObject *
_imp_release_lock_impl(PyObject *module) {
  // TODO follow cpy
  Py_RETURN_NONE;
}

static PyObject *
_imp_release_lock(PyObject *module, PyObject *ignored) {
  return _imp_release_lock_impl(module);
}

#define _IMP_CREATE_BUILTIN_METHODDEF \
  {"create_builtin", (PyCFunction) _imp_create_builtin, METH_O, ""},

#define _IMP_EXEC_BUILTIN_METHODDEF \
  {"exec_builtin", (PyCFunction)_imp_exec_builtin, METH_O, ""},

#define _IMP_ACQUIRE_LOCK_METHODDEF \
  {"acquire_lock", (PyCFunction) _imp_acquire_lock, METH_NOARGS, ""},

#define _IMP_RELEASE_LOCK_METHODDEF \
  {"release_lock", (PyCFunction) _imp_release_lock, METH_NOARGS, "" },

static PyObject *
get_frozen_object(PyObject *name) {
  const struct _frozen *p = find_frozen(name);
  int size;

  if (p == NULL) {
    fail(0);
  }
  if (p->code == NULL) {
    fail(0);
  }
  size = p->size;
  if (size < 0)
    size = -size;
  return PyMarshal_ReadObjectFromString((const char *) p->code, size);
}

static PyObject *
_imp_get_frozen_object_impl(PyObject *module, PyObject *name) {
  return get_frozen_object(name);
}

static PyObject *
_imp_get_frozen_object(PyObject *module, PyObject *arg) {
  PyObject *return_value = NULL;
  PyObject *name;

  if (!PyUnicode_Check(arg)) {
    fail(0);
  }
  if (PyUnicode_READY(arg) == -1) {
    fail(0);
  }
  name = arg;
  return_value = _imp_get_frozen_object_impl(module, name);
  return return_value;
}

#define _IMP_GET_FROZEN_OBJECT_METHODDEF \
  {"get_frozen_object", (PyCFunction) _imp_get_frozen_object, METH_O, ""},

static PyObject *
_imp_extension_suffixes_impl(PyObject *module) {
  // TODO: don't support extension for now
  PyObject *list;
  list = PyList_New(0);
  if (list == NULL)
    return NULL;

  return list;
}

static PyObject *
_imp_extension_suffixes(PyObject *module, PyObject *ignored) {
  return _imp_extension_suffixes_impl(module);
}

#define _IMP_EXTENSION_SUFFIXES_METHODDEF \
  {"extension_suffixes", (PyCFunction) _imp_extension_suffixes, METH_NOARGS, ""},

static PyMethodDef imp_methods[] = {
  _IMP_IS_FROZEN_METHODDEF
  _IMP_IS_FROZEN_PACKAGE_METHODDEF
  _IMP_IS_BUILTIN_METHODDEF
  _IMP_CREATE_BUILTIN_METHODDEF
  _IMP_EXEC_BUILTIN_METHODDEF
  _IMP_ACQUIRE_LOCK_METHODDEF
  _IMP_RELEASE_LOCK_METHODDEF
  _IMP_GET_FROZEN_OBJECT_METHODDEF
  _IMP_EXTENSION_SUFFIXES_METHODDEF

  {NULL, NULL}
};

static int imp_module_exec(PyObject *module);

static PyModuleDef_Slot imp_slots[] = {
  {Py_mod_exec, imp_module_exec},
  {0, NULL}
};

static struct PyModuleDef imp_module = {
  PyModuleDef_HEAD_INIT,
  .m_name = "_imp",
  .m_doc = "",
  .m_size = 0,
  .m_methods = imp_methods,
  .m_slots = imp_slots,
};

PyMODINIT_FUNC
PyInit__imp(void) {
  return PyModuleDef_Init(&imp_module);
}

int
_PyImport_SetModuleString(const char *name, PyObject *m) {
  PyInterpreterState *interp = _PyInterpreterState_GET();
  PyObject *modules = interp->modules;
  return PyMapping_SetItemString(modules, name, m);
}

static int
is_builtin(PyObject *name) {
  int i;
  for (i = 0; PyImport_Inittab[i].name != NULL; i++) {
    if (_PyUnicode_EqualToASCIIString(name, PyImport_Inittab[i].name)) {
      if (PyImport_Inittab[i].initfunc == NULL)
        return -1;
      else
        return 1;
    }
  }
  return 0;
}

static PyObject *
_imp_is_builtin_impl(PyObject *module, PyObject *name) {
  return PyLong_FromLong(is_builtin(name));
}

static PyObject *_imp_is_builtin(PyObject *module, PyObject *arg) {
  PyObject *return_value = NULL;
  PyObject *name;

  if (!PyUnicode_Check(arg)) {
    fail(0);
  }
  if (PyUnicode_READY(arg) == -1) {
    fail(0);
  }
  name = arg;
  return_value = _imp_is_builtin_impl(module, name);

exit:
  return return_value;
}

static PyObject *_imp_create_builtin(PyObject *module, PyObject *spec) {
  PyThreadState *tstate = _PyThreadState_GET();

  PyObject *name = PyObject_GetAttrString(spec, "name");
  if (name == NULL) {
    return NULL;
  }
  PyObject *mod = create_builtin(tstate, name, spec);
  Py_DECREF(name);
  return mod;
}

static int
_imp_exec_builtin_impl(PyObject *module, PyObject *mod) {
  return exec_builtin_or_dynamic(mod);
}

static PyObject *_imp_exec_builtin(PyObject *module, PyObject *mod) {
  PyObject *return_value = NULL;
  int _return_value;

  _return_value = _imp_exec_builtin_impl(module, mod);
  if ((_return_value == -1) && PyErr_Occurred()) {
    goto exit;
  }
  return_value = PyLong_FromLong((long)_return_value);

exit:
  return return_value;
}

int
_PyImport_IsInitialized(PyInterpreterState *interp) {
  if (interp->modules == NULL)
    return 0;
  return 1;
}

PyStatus
_PyImportZip_Init(PyThreadState *tstate) {
  // TODO initialize zip importer
  return _PyStatus_OK();
}
