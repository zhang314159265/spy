#pragma once

// independently from the Python version
#define PYTHON_API_VERSION 1013

#define PYTHON_ABI_VERSION 3

PyTypeObject PyModuleDef_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "moduledef",
	.tp_basicsize = sizeof(struct PyModuleDef),
	.tp_itemsize = 0,
};

typedef struct _PyArg_Parser {
  const char *format;
  const char * const *keywords;
  const char *fname;
  const char *custom_msg;
} _PyArg_Parser;

static Py_ssize_t max_module_number;

PyObject *
PyModuleDef_Init(struct PyModuleDef *def) {
	if (PyType_Ready(&PyModuleDef_Type) < 0)
		return NULL;

	if (def->m_base.m_index == 0) {
		max_module_number++;
		Py_SET_REFCNT(def, 1);
		Py_SET_TYPE(def, &PyModuleDef_Type);
		def->m_base.m_index = max_module_number;
	}
	return (PyObject*) def;
}

static int
check_api_version(const char *name, int module_api_version) {
	if (module_api_version != PYTHON_API_VERSION && module_api_version != PYTHON_ABI_VERSION) {
		assert(false);
	}
	return 1;
}

extern PyTypeObject PyModule_Type;

static int
module_init_dict(PyModuleObject *mod, PyObject *md_dict,
		PyObject *name, PyObject *doc) {
	_Py_IDENTIFIER(__name__);

	if (md_dict == NULL)
		return -1;
	if (doc == NULL)
		doc = Py_None;

	if (_PyDict_SetItemId(md_dict, &PyId___name__, name) != 0)
		return -1;
	if (PyUnicode_CheckExact(name)) {
		Py_INCREF(name);
		Py_XSETREF(mod->md_name, name);
	}

	return 0;
}

PyObject *
PyModule_NewObject(PyObject *name) {
	PyModuleObject *m;
	m = PyObject_GC_New(PyModuleObject, &PyModule_Type);
	if (m == NULL)
		return NULL;
	m->md_dict = PyDict_New();
	if (module_init_dict(m, m->md_dict, name, NULL) != 0)
		assert(false);
	// PyObject_GC_Track(m);
	return (PyObject *) m;
}

PyObject *
PyModule_New(const char *name) {
	PyObject *nameobj, *module;
	nameobj = PyUnicode_FromString(name);
	if (nameobj == NULL)
		return NULL;
	module = PyModule_NewObject(nameobj);
	Py_DECREF(nameobj);
	return module;
}

PyObject *
PyModule_GetNameObject(PyObject *m) {
	_Py_IDENTIFIER(__name__);
	PyObject *d;
	PyObject *name;

	if (!PyModule_Check(m)) {
		assert(false);
	}
	d = ((PyModuleObject *)m)->md_dict;
	if (d == NULL || !PyDict_Check(d) ||
			(name = _PyDict_GetItemIdWithError(d, &PyId___name__)) == NULL ||
			!PyUnicode_Check(name)) {
		assert(false);
	}
	Py_INCREF(name);
	return name;
}

static int
_add_methods_to_object(PyObject *module, PyObject *name, PyMethodDef *functions) {
	PyObject *func;
	PyMethodDef *fdef;

	for (fdef = functions; fdef->ml_name != NULL; fdef++) {
		func = PyCFunction_NewEx(fdef, (PyObject*) module, name);
		if (func == NULL) {
			return -1;
		}
	
		if (PyObject_SetAttrString(module, fdef->ml_name, func) != 0) {
			Py_DECREF(func);
			return -1;
		}
		Py_DECREF(func);
	}

	return 0;
}

int
PyModule_AddFunctions(PyObject *m, PyMethodDef *functions) {
	int res;
	PyObject *name = PyModule_GetNameObject(m);
	if (name == NULL) {
		return -1;
	}

	res = _add_methods_to_object(m, name, functions);
	Py_DECREF(name);
	return res;
}

int
PyModule_SetDocString(PyObject *m, const char *doc) {
	_Py_IDENTIFIER(__doc__);
	PyObject *v;

	v = PyUnicode_FromString(doc);
	if (v == NULL || _PyObject_SetAttrId(m, &PyId___doc__, v) != 0) {
		Py_XDECREF(v);
		return -1;
	}
	Py_DECREF(v);
	return 0;
}

// defined in cpy/Objects/moduleobject.c
PyObject * _PyModule_CreateInitialized(struct PyModuleDef* module, int module_api_version) {
	const char *name;
	PyModuleObject *m;

	if (!PyModuleDef_Init(module))
		return NULL;

	name = module->m_name;
	printf("_PyModule_CreateInitialized module %s\n", name);

	if (!check_api_version(name, module_api_version)) {
		return NULL;
	}

	if (module->m_slots) {
		assert(false);
	}

	if ((m = (PyModuleObject*) PyModule_New(name)) == NULL)
		return NULL;

	if (module->m_size > 0) {
		assert(false);
	}

	if (module->m_methods != NULL) {
		if (PyModule_AddFunctions((PyObject *) m, module->m_methods) != 0) {
			Py_DECREF(m);
			return NULL;
		}
	}
	if (module->m_doc != NULL) {
		if (PyModule_SetDocString((PyObject *) m, module->m_doc) != 0) {
			Py_DECREF(m);
			return NULL;
		}
	}
	m->md_def = module;
	return (PyObject *) m;
}

int PyArg_ParseTuple(PyObject *, const char *, ...);

int _PyArg_NoKeywords(const char *funcname, PyObject *kwargs);

#define _PyArg_NoKeywords(funcname, kwargs) \
  ((kwargs) == NULL || _PyArg_NoKeywords((funcname), (kwargs)))

PyObject *const * _PyArg_UnpackKeywords(
    PyObject *const *args, Py_ssize_t nargs,
    PyObject *kwargs, PyObject *kwnames,
    struct _PyArg_Parser *parser,
    int minpos, int maxpos, int minkw,
    PyObject **buf) {
  assert(false);
}

#define _PyArg_UnpackKeywords(args, nargs, kwargs, kwnames, parser, minpos, maxpos, minkw, buf) \
  (((minkw) == 0 && (kwargs) == NULL && (kwnames) == NULL && \
    (minpos) <= (nargs) && (nargs) <= (maxpos) && args != NULL) ? (args) : \
  _PyArg_UnpackKeywords((args), (nargs), (kwargs), (kwnames), (parser), \
    (minpos), (maxpos), (minkw), (buf)))

int _PyArg_NoKwnames(const char *funcname, PyObject *kwnnames);
