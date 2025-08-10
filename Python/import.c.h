#include "namespaceobject.h"

PyObject *
_PyImport_BootstrapImp(PyThreadState *tstate) {
  PyObject *name = PyUnicode_FromString("_imp");
  if (name == NULL) {
    return NULL;
  }

  PyObject *attrs = Py_BuildValue("{sO}", "name", name);
  if (attrs == NULL) {
    assert(false);
  }

  PyObject *spec = _PyNamespace_New(attrs);
  Py_DECREF(attrs);
  if (spec == NULL) {
    assert(false);
  }

  PyObject *mod = create_builtin(tstate, name, spec);
  Py_CLEAR(name);
  Py_DECREF(spec);
  if (mod == NULL) {
    assert(false);
  }
  assert(mod != Py_None);

  if (exec_builtin_or_dynamic(mod) < 0) {
    assert(false);
  }
  return mod;
}

int
_PyImport_FixupExtensionObject(PyObject *mod, PyObject *name,
    PyObject *filename, PyObject *modules) {
  if (mod == NULL || !PyModule_Check(mod)) {
    fail(0);
  }

  struct PyModuleDef *def = PyModule_GetDef(mod);
  if (!def) {
    fail(0);
  }

  PyThreadState *tstate = _PyThreadState_GET();
  if (PyObject_SetItem(modules, name, mod) < 0) {
    return -1;
  }
  if (_PyState_AddModule(tstate, mod, def) < 0) {
    fail(0);
  }

  if (_Py_IsMainInterpreter(tstate->interp) || def->m_size == -1) {
    if (def->m_size == -1) {
      if (def->m_base.m_copy) {
        fail(0);
      }
      PyObject *dict = PyModule_GetDict(mod);
      if (dict == NULL) {
        return -1;
      }
      def->m_base.m_copy = PyDict_Copy(dict);
      if (def->m_base.m_copy == NULL) {
        return -1;
      }
    }

    if (extensions == NULL) {
      extensions = PyDict_New();
      if (extensions == NULL) {
        return -1;
      }
    }

    PyObject *key = PyTuple_Pack(2, filename, name);
    if (key == NULL) {
      return -1;
    }
    int res = PyDict_SetItem(extensions, key, (PyObject *) def);
    Py_DECREF(key);
    if (res < 0) {
      return -1;
    }
  }
  return 0;
}


int _PyImport_FixupBuiltin(PyObject *mod, const char *name, PyObject *modules) {
  int res;
  PyObject *nameobj;
  nameobj = PyUnicode_InternFromString(name);
  if (nameobj == NULL) {
    return -1;
  }
  res = _PyImport_FixupExtensionObject(mod, nameobj, nameobj, modules);
  Py_DECREF(nameobj);
  return res;
}

static PyObject *
create_builtin(PyThreadState *tstate, PyObject *name, PyObject *spec) {
  PyObject *mod = import_find_extension(tstate, name, name);
  if (mod || _PyErr_Occurred(tstate)) {
    return mod;
  }

  PyObject *modules = tstate->interp->modules;
  for (struct _inittab *p = PyImport_Inittab; p->name != NULL; p++) {
    if (_PyUnicode_EqualToASCIIString(name, p->name)) {
      if (p->initfunc == NULL) {
        // Cannot re-init internal module ("sys" or "builtins")
        mod = PyImport_AddModuleObject(name);
        return Py_XNewRef(mod);
      }

      mod = (*p->initfunc)();
      if (mod == NULL) {
        return NULL;
      }

      if (PyObject_TypeCheck(mod, &PyModuleDef_Type)) {
        return PyModule_FromDefAndSpec((PyModuleDef *) mod, spec);
      } else {
        // Remember pointer to module init function
        PyModuleDef *def = PyModule_GetDef(mod);
        if (def == NULL) {
          return NULL;
        }

        def->m_base.m_init = p->initfunc;
        if (_PyImport_FixupExtensionObject(mod, name, name,
            modules) < 0) {
          return NULL;
        }
        return mod;
      }
    }
  }

  // not found
  fail("not found %s", (char *) PyUnicode_DATA(name)); // should return None instead
}

static int imp_module_exec(PyObject *module) {
  const wchar_t *mode = _Py_GetConfig()->check_hash_pycs_mode;
  PyObject *pyc_mode = PyUnicode_FromWideChar(mode, -1);
  if (pyc_mode == NULL) {
    fail(0);
    return -1;
  }
  if (PyModule_AddObjectRef(module, "check_hash_based_pycs", pyc_mode) < 0) {
    fail(0);
  }
  Py_DECREF(pyc_mode);
  return 0;
}

static PyObject *
_imp_is_frozen_impl(PyObject *module, PyObject *name) {
  const struct _frozen *p;
  p = find_frozen(name);
  return PyBool_FromLong((long) (p == NULL ? 0 : p->size));
}

static PyObject *_imp_is_frozen(PyObject *module, PyObject *arg) {
  PyObject *return_value = NULL;
  PyObject *name;

  if (!PyUnicode_Check(arg)) {
    fail(0);
  }
  if (PyUnicode_READY(arg) == -1) {
    fail(0);
  }
  name = arg;
  return_value = _imp_is_frozen_impl(module, name);

  return return_value;
}

PyObject *PyImport_Import(PyObject *module_name) {
	_Py_IDENTIFIER(__import__);
	_Py_IDENTIFIER(__builtin__);

	PyObject *globals = NULL;
  PyObject *import = NULL;
	PyObject *builtins = NULL;
  PyObject *r = NULL;

	PyThreadState *tstate = _PyThreadState_GET();

	PyObject *import_str = _PyUnicode_FromId(&PyId___import__);
	if (import_str == NULL) {
		return NULL;
	}

  PyObject *builtins_str = _PyUnicode_FromId(&PyId___builtins__);
  if (builtins_str == NULL)
    return NULL;

  PyObject *from_list = PyList_New(0);
  if (from_list == NULL) {
    fail(0);
  }

	// Get the builtins from current globals
	globals = PyEval_GetGlobals();
	if (globals != NULL) {
    Py_INCREF(globals);
    builtins = PyObject_GetItem(globals, builtins_str);
    if (builtins == NULL)
      fail(0);
	} else {
		builtins = PyImport_ImportModuleLevel("builtins",
				NULL, NULL, NULL, 0);
		if (builtins == NULL) {
			assert(false);
		}
		assert(false);
	}

  if (PyDict_Check(builtins)) {
    import = PyObject_GetItem(builtins, import_str);
    if (import == NULL) {
      fail(0);
    }
  } else {
    fail(0);
  }
  if (import == NULL) {
    fail(0);
  }

  // calling for side-effect of import
  r = PyObject_CallFunction(import, "OOOOi", module_name, globals,
      globals, from_list, 0, NULL);
  if (r == NULL)
    fail(0);
  Py_DECREF(r);

  r = import_get_module(tstate, module_name);
  if (r == NULL && !_PyErr_Occurred(tstate)) {
    fail(0);
  }
err:
  Py_XDECREF(globals);
  Py_XDECREF(builtins);
  Py_XDECREF(import);
  Py_XDECREF(from_list);

  return r;
}

