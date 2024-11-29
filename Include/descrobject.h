#pragma once

#define PyWrapperFlag_KEYWORDS 1  // wrapper function takes keyword args

typedef PyObject *(*wrapperfunc)(PyObject *self, PyObject *args,
    void *wrapped);

struct wrapperbase {
  const char *name;
  int offset;
  void *function;
  wrapperfunc wrapper;
  const char *doc;
  int flags;
  PyObject *name_strobj;
};

typedef struct {
	PyObject_HEAD
	PyTypeObject *d_type;
	PyObject *d_name;
	PyObject *d_qualname;
} PyDescrObject;

#define PyDescr_COMMON PyDescrObject d_common

#define PyDescr_TYPE(x) (((PyDescrObject *) (x))->d_type)

typedef PyObject *(*getter)(PyObject *, void *);
typedef int (*setter)(PyObject *, PyObject *, void *);

typedef struct PyGetSetDef {
  const char *name;
  getter get;
  setter set;
  const char *doc;
  void *closure;
} PyGetSetDef;

typedef struct {
	PyDescr_COMMON;
	PyMethodDef *d_method;
	vectorcallfunc vectorcall;
} PyMethodDescrObject;

typedef struct {
  PyDescr_COMMON;
  struct wrapperbase *d_base;
  void *d_wrapped;
} PyWrapperDescrObject;

typedef struct {
  PyDescr_COMMON;
  PyGetSetDef *d_getset;
} PyGetSetDescrObject;

typedef void (*funcptr)(void);

static inline funcptr
method_enter_call(PyThreadState *tstate, PyObject *func) {
	return (funcptr) ((PyMethodDescrObject *) func)->d_method->ml_meth;
}

static PyObject *
method_vectorcall_VARARGS_KEYWORDS(
		PyObject *func, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
	PyThreadState *tstate = _PyThreadState_GET();
	Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);

	PyObject *argstuple = _PyTuple_FromArray(args + 1, nargs - 1);
	if (argstuple == NULL) {
		return NULL;
	}
	PyObject *result = NULL;
	PyObject *kwdict = NULL;
	if (kwnames != NULL && PyTuple_GET_SIZE(kwnames) > 0) {
		assert(false);
	}

	PyCFunctionWithKeywords meth = (PyCFunctionWithKeywords)
			method_enter_call(tstate, func);
	if (meth == NULL) {
		assert(false);
	}
	result = meth(args[0], argstuple, kwdict);
exit:
	Py_DECREF(argstuple);
	Py_XDECREF(kwdict);
	return result;
}

static PyObject *
classmethoddescr_call(PyMethodDescrObject *descr, PyObject *args,
    PyObject *kwds) {
  assert(false);
}

PyTypeObject PyMethodDescr_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "method_descriptor",
	.tp_basicsize = sizeof(PyMethodDescrObject),
	.tp_flags = Py_TPFLAGS_HAVE_VECTORCALL | Py_TPFLAGS_METHOD_DESCRIPTOR,
	.tp_call = PyVectorcall_Call,
	.tp_vectorcall_offset = offsetof(PyMethodDescrObject, vectorcall),
};

static PyObject * classmethod_get(PyMethodDescrObject *descr, PyObject *obj, PyObject *type);

PyTypeObject PyClassMethodDescr_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "classmethod_descriptor",
  .tp_basicsize = sizeof(PyMethodDescrObject),
  .tp_call = (ternaryfunc) classmethoddescr_call,
  .tp_descr_get = (descrgetfunc) classmethod_get,
  .tp_descr_set = 0,
};

PyTypeObject PyWrapperDescr_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "wrapper_descriptor",
  .tp_basicsize = sizeof(PyWrapperDescrObject),
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC |
      Py_TPFLAGS_METHOD_DESCRIPTOR,
};

static void descr_dealloc(PyDescrObject *descr) {
  assert(false);
}

static PyObject *getset_get(PyGetSetDescrObject *descr, PyObject *obj, PyObject *type);

static int getset_set(PyGetSetDescrObject *descr, PyObject *obj, PyObject *value);

PyTypeObject PyGetSetDescr_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "getset_descriptor",
  .tp_basicsize = sizeof(PyGetSetDescrObject),
  .tp_dealloc = (destructor) descr_dealloc,
  .tp_getattro = PyObject_GenericGetAttr,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
  .tp_descr_get = (descrgetfunc) getset_get,
  .tp_descr_set = (descrsetfunc) getset_set,
};

static PyDescrObject *
descr_new(PyTypeObject *descrtype, PyTypeObject *type, const char *name) {
	PyDescrObject *descr;

	descr = (PyDescrObject *) PyType_GenericAlloc(descrtype, 0);
	if (descr != NULL) {
		Py_XINCREF(type);
		descr->d_type = type;
		descr->d_name = PyUnicode_InternFromString(name);
		if (descr->d_name == NULL) {
			Py_DECREF(descr);
			descr = NULL;
		} else {
			descr->d_qualname = NULL;
		}
	}
	return descr;
}

static PyObject *
method_vectorcall_O(
		PyObject *func, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
	PyThreadState *tstate = _PyThreadState_GET();
	Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
	if (nargs != 2) {
		assert(false);
	}
	PyCFunction meth = (PyCFunction) method_enter_call(tstate, func);
	if (meth == NULL) {
		return NULL;
	}
	PyObject *result = meth(args[0], args[1]);
	return result;
}

PyObject *
PyDescr_NewMethod(PyTypeObject *type, PyMethodDef *method) {
	vectorcallfunc vectorcall;
	switch (method->ml_flags & (METH_VARARGS | METH_FASTCALL | METH_NOARGS |
			METH_O | METH_KEYWORDS | METH_METHOD)) {
	case METH_VARARGS | METH_KEYWORDS:
		vectorcall = method_vectorcall_VARARGS_KEYWORDS;
		break;
	case METH_O:
		vectorcall = method_vectorcall_O;
		break;
	default:
		assert(false);
	}

	PyMethodDescrObject *descr;

	descr = (PyMethodDescrObject *) descr_new(
			&PyMethodDescr_Type, type, method->ml_name);
	if (descr != NULL) {
		descr->d_method = method;
		descr->vectorcall = vectorcall;
	}
	return (PyObject *) descr;
}

#define PyDescr_NAME(x) (((PyDescrObject *)(x))->d_name)

PyObject *
PyDescr_NewClassMethod(PyTypeObject *type, PyMethodDef *method) {
  PyMethodDescrObject *descr;

  descr = (PyMethodDescrObject *) descr_new(
      &PyClassMethodDescr_Type,
      type, method->ml_name);
  if (descr != NULL)
    descr->d_method = method;
  return (PyObject *) descr;
}

static PyObject * classmethod_get(PyMethodDescrObject *descr, PyObject *obj, PyObject *type) {
  if (type == NULL) {
    assert(false);
  }
  if (!PyType_Check(type)) {
    assert(false);
  }
  if (!PyType_IsSubtype((PyTypeObject *) type, PyDescr_TYPE(descr))) {
    assert(false);
  }
  PyTypeObject *cls = NULL;
  if (descr->d_method->ml_flags & METH_METHOD) {
    assert(false);
  }
  return PyCMethod_New(descr->d_method, type, NULL, cls);
}

int
PyDescr_IsData(PyObject *ob) {
  return Py_TYPE(ob)->tp_descr_set != NULL;
}

PyObject *
PyDescr_NewWrapper(PyTypeObject *type, struct wrapperbase *base, void *wrapped) {
  PyWrapperDescrObject *descr;

  descr = (PyWrapperDescrObject *) descr_new(&PyWrapperDescr_Type,
      type, base->name);

  if (descr != NULL) {
    descr->d_base = base;
    descr->d_wrapped = wrapped;
  }
  return (PyObject *) descr;
}

PyObject *
PyDescr_NewGetSet(PyTypeObject *type, PyGetSetDef *getset) {
  // printf("PyDescr_NewGetSet for type %s\n", type->tp_name);
  PyGetSetDescrObject *descr;

  descr = (PyGetSetDescrObject *) descr_new(&PyGetSetDescr_Type,
      type, getset->name);
  if (descr != NULL)
    descr->d_getset = getset;
  return (PyObject *) descr;
}

static int
descr_check(PyDescrObject *descr, PyObject *obj) {
  if (!PyObject_TypeCheck(obj, descr->d_type)) {
    assert(false);
  }
  return 0;
}

static PyObject *getset_get(PyGetSetDescrObject *descr, PyObject *obj, PyObject *type) {
  if (obj == NULL) {
    assert(false);
  }
  if (descr_check((PyDescrObject *) descr, obj) < 0) {
    return NULL;
  }
  if (descr->d_getset->get != NULL) {
    return descr->d_getset->get(obj, descr->d_getset->closure);
  }
  assert(false);
}

static int getset_set(PyGetSetDescrObject *descr, PyObject *obj, PyObject *value) {
  assert(false);
}
