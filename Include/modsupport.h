#pragma once

// independently from the Python version
#define PYTHON_API_VERSION 1013

#define PYTHON_ABI_VERSION 3

#define Py_CLEANUP_SUPPORTED 0x20000

typedef struct _PyArg_Parser {
  const char *format;
  const char * const *keywords;
  const char *fname;
  const char *custom_msg;
  int pos;
  PyObject *kwtuple;
  struct _PyArg_Parser *next;
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
  m->md_def = NULL;
  m->md_state = NULL;
  m->md_weaklist = NULL;
  m->md_name = NULL;
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
    m->md_state = PyMem_Malloc(module->m_size);
    if (!m->md_state) {
      fail(0);
    }
    memset(m->md_state, 0, module->m_size);
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

int parser_init(struct _PyArg_Parser *parser);
PyObject *find_keyword(PyObject *kwnames, PyObject *const *kwstack, PyObject *key);

PyObject *const * _PyArg_UnpackKeywords(
    PyObject *const *args, Py_ssize_t nargs,
    PyObject *kwargs, PyObject *kwnames,
    struct _PyArg_Parser *parser,
    int minpos, int maxpos, int minkw,
    PyObject **buf) {
  PyObject *kwtuple;
  PyObject *keyword;
  int i, posonly, minposonly, maxargs;
  int reqlimit = minkw ? maxpos + minkw : minpos;
  Py_ssize_t nkwargs;
  PyObject *current_arg;
  PyObject *const *kwstack = NULL;

  assert(kwargs == NULL || PyDict_Check(kwargs));
  assert(kwargs == NULL || kwnames == NULL);

  if (parser == NULL) {
    fail(0);
  }

  if (kwnames != NULL && !PyTuple_Check(kwnames)) {
    fail(0);
  }

  if (args == NULL && nargs == 0) {
    args = buf;
  }

  if (!parser_init(parser)) {
    return NULL;
  }

  kwtuple = parser->kwtuple;
  posonly = parser->pos;
  minposonly = Py_MIN(posonly, minpos);
  maxargs = posonly + (int) PyTuple_GET_SIZE(kwtuple);

  if (kwargs != NULL) {
    fail(0);
  } else if (kwnames != NULL) {
    nkwargs = PyTuple_GET_SIZE(kwnames);
    kwstack = args + nargs;
  } else {
    nkwargs = 0;
  }
  if (nkwargs == 0 && minkw == 0 && minpos <= nargs && nargs <= maxpos) {
    // Fast path.
    return args;
  }

  if (nargs + nkwargs > maxargs) {
    fail(0);
  }
  if (nargs > maxpos) {
    fail(0);
  }
  if (nargs < minposonly) {
    fail(0);
  }

  // copy tuple args
  for (i = 0; i < nargs; i++) {
    buf[i] = args[i];
  }

  for (i = Py_MAX((int) nargs, posonly); i < maxargs; i++) {
    if (nkwargs) {
      keyword = PyTuple_GET_ITEM(kwtuple, i - posonly);
      if (kwargs != NULL) {
        fail(0);
      } else {
        current_arg = find_keyword(kwnames, kwstack, keyword);
      }
    } else if (i >= reqlimit) {
      break;
    } else {
      current_arg = NULL;
    }

    buf[i] = current_arg;

    if (current_arg) {
      --nkwargs;
    } else if (i < minpos || (maxpos <= i && i < reqlimit)) {
      fail(0);
    }
  }

  if (nkwargs > 0) {
    fail(0);
  }
  return buf;
}

int PyArg_UnpackTuple(PyObject *, const char *, Py_ssize_t, Py_ssize_t, ...);

#define _PyArg_UnpackKeywords(args, nargs, kwargs, kwnames, parser, minpos, maxpos, minkw, buf) \
  (((minkw) == 0 && (kwargs) == NULL && (kwnames) == NULL && \
    (minpos) <= (nargs) && (nargs) <= (maxpos) && args != NULL) ? (args) : \
  _PyArg_UnpackKeywords((args), (nargs), (kwargs), (kwnames), (parser), \
    (minpos), (maxpos), (minkw), (buf)))

int _PyArg_NoKwnames(const char *funcname, PyObject *kwnnames);
static Py_ssize_t
countformat(const char *format, char endchar) {
  Py_ssize_t count = 0;
  int level = 0;

  while (level > 0 || *format != endchar) {
    switch (*format) {
    case '(':
    case '{':
      if (level == 0) {
        count++;
      }
      level++;
      break;
    case ')':
    case '}':
      level--;
      break;
    case 's':
    case 'O':
      if (level == 0) {
        count++;
      }
      break;

    // these are default cases
    case 'i':
      if (level == 0) {
        count++;
      }
      break;
      
    default:
      fail("can not handle format character %c\n", *format);
    }
    format++;
  }
  return count;
}

static PyObject *do_mkdict(const char **p_format, va_list *p_va, char endchar, Py_ssize_t n, int flags);

static PyObject *do_mktuple(const char **p_format, va_list *p_va, char endchar, Py_ssize_t n, int flags);

static PyObject *
do_mkvalue(const char **p_format, va_list *p_va, int flags) {
  for (;;) {
    char ch = *(*p_format)++;
    switch (ch) {
    case 'i':
      return PyLong_FromLong((long) va_arg(*p_va, int));
    case 's':
    {
      PyObject *v;
      const char *str = va_arg(*p_va, const char *);
      Py_ssize_t n;
      if (**p_format == '#') {
        fail(0);
      } else 
        n = -1;
      if (str == NULL) {
        fail(0);
      } else {
        if (n < 0) {
          size_t m = strlen(str);
          n = (Py_ssize_t) m;
        }
        v = PyUnicode_FromStringAndSize(str, n);
      }
      return v;
    }
    case '(':
      return do_mktuple(p_format, p_va, ')',
          countformat(*p_format, ')'), flags);
    case '{':
      return do_mkdict(p_format, p_va, '}',
          countformat(*p_format, '}'), flags);
    case 'O':
      if (**p_format == '&') {
        fail(0);
      }
      else {
        PyObject *v;
        v = va_arg(*p_va, PyObject *);
        if (v != NULL) {
          if (*(*p_format - 1) != 'N')
            Py_INCREF(v);
        } else {
          fail(0);
        }
        return v;
      }
    default:
      fail("can not handle format character '%c'", ch);
    }
  }
}

static PyObject *
va_build_value(const char *format, va_list va, int flags) {
  const char *f = format;
  Py_ssize_t n = countformat(f, '\0');
  va_list lva;
  PyObject *retval;

  if (n < 0)
    return NULL;
  if (n == 0)
    Py_RETURN_NONE;
  va_copy(lva, va);
  if (n == 1) {
    retval = do_mkvalue(&f, &lva, flags);
  } else {
    fail("multi value");
  }
  va_end(lva);
  return retval;
}

PyObject *
Py_BuildValue(const char *format, ...) {
  va_list va;
  PyObject *retval;
  va_start(va, format);
  retval = va_build_value(format, va, 0);
  va_end(va);
  return retval;
}


static PyObject *do_mkdict(const char **p_format, va_list *p_va, char endchar, Py_ssize_t n, int flags) {
  PyObject *d;
  Py_ssize_t i;
  if (n < 0)
    return NULL;
  if (n % 2) {
    fail("");
  }
  if ((d = PyDict_New()) == NULL) {
    fail("");
  }
  for (i = 0; i < n; i += 2) {
    PyObject *k, *v;

    k = do_mkvalue(p_format, p_va, flags);
    if (k == NULL) {
      fail("");
    }
    v = do_mkvalue(p_format, p_va, flags);
    if (v == NULL || PyDict_SetItem(d, k, v) < 0) {
      fail("");
    }
    Py_DECREF(k);
    Py_DECREF(v);
  }

  if (**p_format != endchar) {
    fail("");
  }
  if (endchar)
    ++*p_format;
  return d;
}

PyObject *PyModule_FromDefAndSpec2(PyModuleDef *def,
    PyObject *spec,
    int module_api_version);

#define PyModule_FromDefAndSpec(module, spec) \
  PyModule_FromDefAndSpec2(module, spec, PYTHON_API_VERSION)


int PyModule_AddObjectRef(PyObject *mod, const char *name, PyObject *value) {
  if (!PyModule_Check(mod)) {
    fail(0);
  }
  if (!value) {
    fail(0);
  }

  PyObject *dict = PyModule_GetDict(mod);
  if (dict == NULL) {
    fail(0);
  }

  if (PyDict_SetItemString(dict, name, value)) {
    return -1;
  }
  return 0;
}

int PyModule_AddObject(PyObject *mod, const char *name, PyObject *value) {
  int res = PyModule_AddObjectRef(mod, name, value);
  if (res == 0) {
    Py_DECREF(value);
  }
  return res;
}

static int
do_mkstack(PyObject **stack, const char **p_format, va_list *p_va,
    char endchar, Py_ssize_t n, int flags) {
  Py_ssize_t i;

  if (n < 0) {
    return -1;
  }

  for (i = 0; i < n; i++) {
    PyObject *w = do_mkvalue(p_format, p_va, flags);
    if (w == NULL) {
      fail(0);
    }
    stack[i] = w;
  }
  if (**p_format != endchar) {
    fail(0);
  }
  if (endchar) {
    ++*p_format;
  }
  return 0;
}

static PyObject **
va_build_stack(PyObject **small_stack, Py_ssize_t small_stack_len,
    const char *format, va_list va, int flags, Py_ssize_t *p_nargs) {
  const char *f;
  Py_ssize_t n;
  va_list lva;
  PyObject **stack;
  int res;

  n = countformat(format, '\0');
  if (n < 0) {
    fail(0);
  }

  if (n == 0) {
    fail(0);
  }

  if (n <= small_stack_len) {
    stack = small_stack;
  } else {
    fail(0);
  }

  va_copy(lva, va);
  f = format;
  res = do_mkstack(stack, &f, &lva, '\0', n, flags);
  va_end(lva);

  if (res < 0) {
    fail(0);
  }

  *p_nargs = n;
  return stack;
}

PyObject **
_Py_VaBuildStack(PyObject **small_stack, Py_ssize_t small_stack_len,
    const char *format, va_list va, Py_ssize_t *p_nargs) {
  return va_build_stack(small_stack, small_stack_len, format, va, 0, p_nargs);
}

#define PyModule_Create(module) \
  PyModule_Create2(module, PYTHON_API_VERSION)

PyObject *PyModule_Create2(struct PyModuleDef *module,
    int module_api_version) {
  if (!_PyImport_IsInitialized(_PyInterpreterState_GET())) {
    fail(0);
  }
  return _PyModule_CreateInitialized(module, module_api_version);
}


static PyObject *do_mktuple(const char **p_format, va_list *p_va, char endchar, Py_ssize_t n, int flags) {
  PyObject *v;
  Py_ssize_t i;

  if (n < 0) {
    return NULL;
  }

  if ((v = PyTuple_New(n)) == NULL) {
    fail(0);
  }

  for (i = 0; i < n; i++) {
    PyObject *w = do_mkvalue(p_format, p_va, flags);
    if (w == NULL) {
      fail(0);
    }
    PyTuple_SET_ITEM(v, i, w);
  }

  if (**p_format != endchar) {
    fail(0);
  }
  if (endchar)
    ++*p_format;
  return v;
}

int
PyModule_AddType(PyObject *module, PyTypeObject *type) {
  if (PyType_Ready(type) < 0) {
    fail("fail to add type to module due to PyType_Ready fail");
    return -1;
  }

  const char *name = _PyType_Name(type);
  assert(name != NULL);

  return PyModule_AddObjectRef(module, name, (PyObject *) type);
}

int
_Py_convert_optional_to_ssize_t(PyObject *obj, void *result) {
  fail(0);
  Py_ssize_t limit;
  if (obj == Py_None) {
    return 1;
  } else if (_PyIndex_Check(obj)) {
    fail(0);
  } else {
    fail(0);
  }
  *((Py_ssize_t *) result) = limit;
  return 1;
}
