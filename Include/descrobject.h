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
  struct PyMemberDef *d_member;
} PyMemberDescrObject;

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

static int descr_check(PyDescrObject *descr, PyObject *obj);

static inline int
method_check_args(PyObject *func, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames) {
  assert(!PyErr_Occurred());
  if (nargs < 1) {
    fail(0);
  }
  PyObject *self = args[0];
  if (descr_check((PyDescrObject *) func, self) < 0) {
    return -1;
  }
  if (kwnames && PyTuple_GET_SIZE(kwnames)) {
    fail(0);
  }
  return 0;
}

static PyObject *
method_vectorcall_VARARGS(
    PyObject *func, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
  PyThreadState *tstate = _PyThreadState_GET();
  Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
  if (method_check_args(func, args, nargs, kwnames)) {
    return NULL;
  }
  PyObject *argstuple = _PyTuple_FromArray(args + 1, nargs - 1);
  if (argstuple == NULL) {
    return NULL;
  }
  PyCFunction meth = (PyCFunction) method_enter_call(tstate, func);
  if (meth == NULL) {
    Py_DECREF(argstuple);
    return NULL;
  }
  PyObject *result = meth(args[0], argstuple);
  Py_DECREF(argstuple);
  return result;
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


static PyObject *
method_get(PyMethodDescrObject *descr, PyObject *obj, PyObject *type) {
  if (obj == NULL) {
    assert(false);
  }
  if (descr_check((PyDescrObject *) descr, obj) < 0) {
    return NULL;
  }
  if (descr->d_method->ml_flags & METH_METHOD) {
    assert(false);
  } else {
    return PyCFunction_NewEx(descr->d_method, obj, NULL);
  }
}

PyTypeObject PyMethodDescr_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "method_descriptor",
	.tp_basicsize = sizeof(PyMethodDescrObject),
	.tp_flags = Py_TPFLAGS_HAVE_VECTORCALL | Py_TPFLAGS_METHOD_DESCRIPTOR,
	.tp_call = PyVectorcall_Call,
	.tp_vectorcall_offset = offsetof(PyMethodDescrObject, vectorcall),
  .tp_descr_get = (descrgetfunc) method_get,
};

static void
descr_dealloc(PyDescrObject *descr) {
  assert(false);
}

static PyObject *
member_get(PyMemberDescrObject *descr, PyObject *obj, PyObject *type) {
  if (obj == NULL) {
    return Py_NewRef(descr);
  }
  if (descr_check((PyDescrObject *) descr, obj) < 0) {
    return NULL;
  }

  if (descr->d_member->flags & PY_AUDIT_READ) {
    assert(false);
  }

  return PyMember_GetOne((char *) obj, descr->d_member);
}

static int
descr_setcheck(PyDescrObject *descr, PyObject *obj, PyObject *value) {
  assert(obj != NULL);
  if (!PyObject_TypeCheck(obj, descr->d_type)) {
    assert(false);
  }
  return 0;
}

static int
member_set(PyMemberDescrObject *descr, PyObject *obj, PyObject *value) {
  if (descr_setcheck((PyDescrObject *) descr, obj, value) < 0) {
    return -1;
  }
  return PyMember_SetOne((char *) obj, descr->d_member, value);
}

PyTypeObject PyMemberDescr_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "member_descriptor",
  .tp_basicsize = sizeof(PyMemberDescrObject),
  .tp_dealloc = (destructor) descr_dealloc,
  .tp_getattro = PyObject_GenericGetAttr,
  .tp_descr_get = (descrgetfunc) member_get,
  .tp_descr_set = (descrsetfunc) member_set,
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

typedef struct {
  PyObject_HEAD
  PyObject *prop_get;
  PyObject *prop_set;
  PyObject *prop_del;
  PyObject *prop_doc;
  PyObject *prop_name;
  int getter_doc;
} propertyobject;

static void property_dealloc(PyObject *self);

static PyGetSetDef property_getsetlist[] = {
  {NULL}
};

static PyObject *property_descr_get(PyObject *self, PyObject *obj, PyObject *type);
static int property_descr_set(PyObject *self, PyObject *obj, PyObject *value);
static int property_init(PyObject *self, PyObject *args, PyObject *kwargs);
PyObject *PyObject_CallFunctionObjArgs(PyObject *callable, ...);
static PyObject *property_copy(PyObject *old, PyObject *get, PyObject *set, PyObject *del);

static PyObject *
property_setter(PyObject *self, PyObject *setter) {
  return property_copy(self, NULL, setter, NULL);
}

PyDoc_STRVAR(setter_doc, "");

static PyMethodDef property_methods[] = {
  {"setter", property_setter, METH_O, setter_doc},
  {0}
};

PyTypeObject PyProperty_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "property",
  .tp_basicsize = sizeof(propertyobject),
  .tp_dealloc = property_dealloc,
  .tp_getattro = PyObject_GenericGetAttr,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC |
      Py_TPFLAGS_BASETYPE,
  .tp_getset = property_getsetlist,
  .tp_descr_get = property_descr_get,
  .tp_descr_set = property_descr_set,
  .tp_init = property_init,
  .tp_alloc = PyType_GenericAlloc,
  .tp_new = PyType_GenericNew,
  .tp_free = PyObject_GC_Del,
  .tp_methods = property_methods,
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

static PyObject *
method_vectorcall_FASTCALL_KEYWORDS(
    PyObject *func, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
  PyThreadState *tstate = _PyThreadState_GET();
  Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
  if (method_check_args(func, args, nargs, NULL)) {
    return NULL;
  }
  _PyCFunctionFastWithKeywords meth = (_PyCFunctionFastWithKeywords)
      method_enter_call(tstate, func);
  if (meth == NULL) {
    return NULL;
  }
  PyObject *result = meth(args[0], args + 1, nargs - 1, kwnames);
  return result;
}

static PyObject *
method_vectorcall_NOARGS(
    PyObject *func, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
  PyThreadState *tstate = _PyThreadState_GET();
  Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
  if (nargs != 1) {
    fail(0);
  }
  PyCFunction meth = (PyCFunction) method_enter_call(tstate, func);
  if (meth == NULL) {
    return NULL;
  }
  PyObject *result = meth(args[0], NULL);
  return result;
}

static PyObject *
method_vectorcall_FASTCALL(
  PyObject *func, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
  PyThreadState *tstate = _PyThreadState_GET();
  Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);

  _PyCFunctionFast meth = (_PyCFunctionFast) method_enter_call(tstate, func);
  if (meth == NULL) {
    return NULL;
  }
  PyObject *result = meth(args[0], args + 1, nargs - 1);
  return result;
}

PyObject *
PyDescr_NewMethod(PyTypeObject *type, PyMethodDef *method) {
	vectorcallfunc vectorcall;
	switch (method->ml_flags & (METH_VARARGS | METH_FASTCALL | METH_NOARGS |
			METH_O | METH_KEYWORDS | METH_METHOD)) {
  case METH_VARARGS:
    vectorcall = method_vectorcall_VARARGS;
    break;
	case METH_VARARGS | METH_KEYWORDS:
		vectorcall = method_vectorcall_VARARGS_KEYWORDS;
		break;
  case METH_FASTCALL:
    vectorcall = method_vectorcall_FASTCALL;
    break;
	case METH_O:
		vectorcall = method_vectorcall_O;
		break;
  case METH_NOARGS:
    vectorcall = method_vectorcall_NOARGS;
    break;
  case METH_FASTCALL | METH_KEYWORDS:
    vectorcall = method_vectorcall_FASTCALL_KEYWORDS;
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
  if (descr_setcheck((PyDescrObject *) descr, obj, value) < 0) {
    return -1;
  }
  if (descr->d_getset->set != NULL) {
    return descr->d_getset->set(obj, value, descr->d_getset->closure);
  }
  assert(false);
}

static void property_dealloc(PyObject *self) {
  propertyobject *gs = (propertyobject *) self;

  Py_XDECREF(gs->prop_get);
  Py_XDECREF(gs->prop_set);
  Py_XDECREF(gs->prop_del);
  Py_XDECREF(gs->prop_doc);
  Py_XDECREF(gs->prop_name);
  Py_TYPE(self)->tp_free(self);
}

static PyObject *property_descr_get(PyObject *self, PyObject *obj, PyObject *type) {
  if (obj == NULL || obj == Py_None) {
    Py_INCREF(self);
    return self;
  }

  propertyobject *gs = (propertyobject *) self;
  if (gs->prop_get == NULL) {
    assert(false);
  }

  return PyObject_CallOneArg(gs->prop_get, obj);
}

static int property_descr_set(PyObject *self, PyObject *obj, PyObject *value) {
  propertyobject *gs = (propertyobject *) self;
  PyObject *func, *res;

  if (value == NULL)
    func = gs->prop_del;
  else
    func = gs->prop_set;
  if (func == NULL) {
    assert(false);
  }
  if (value == NULL)
    res = PyObject_CallOneArg(func, obj);
  else
    res = PyObject_CallFunctionObjArgs(func, obj, value, NULL);
  if (res == NULL)
    return -1;
  Py_DECREF(res);
  return 0;
}

// defined in cpy/Objects/clinic/descrobject.c.h
static int property_init(PyObject *self, PyObject *args, PyObject *kwargs);

static PyObject *
property_copy(PyObject *old, PyObject *get, PyObject *set, PyObject *del) {
  propertyobject *pold = (propertyobject *) old;
  PyObject *new, *type, *doc;

  type = PyObject_Type(old);
  if (type == NULL)
    return NULL;

  if (get == NULL || get == Py_None) {
    Py_XDECREF(get);
    get = pold->prop_get ? pold->prop_get : Py_None;
  }
  if (set == NULL || set == Py_None) {
    Py_XDECREF(set);
    set = pold->prop_set ? pold->prop_set : Py_None;
  }
  if (del == NULL || del == Py_None) {
    Py_XDECREF(del);
    del = pold->prop_del ? pold->prop_del : Py_None;
  }
  if (pold->getter_doc && get != Py_None) {
    doc = Py_None;
  } else {
    doc = pold->prop_doc ? pold->prop_doc : Py_None;
  }

  new = PyObject_CallFunctionObjArgs(type, get, set, del, doc, NULL);
  Py_DECREF(type);
  if (new == NULL)
    return NULL;
  if (PyObject_TypeCheck((new), &PyProperty_Type)) {
    Py_XINCREF(pold->prop_name);
    Py_XSETREF(((propertyobject *) new)->prop_name, pold->prop_name);
  }
  return new;
}

PyObject *
PyDescr_NewMember(PyTypeObject *type, PyMemberDef *member) {
  PyMemberDescrObject *descr;

  descr = (PyMemberDescrObject *) descr_new(&PyMemberDescr_Type,
      type, member->name);

  if (descr != NULL) {
    descr->d_member = member;
  }

  return (PyObject *) descr;
}
