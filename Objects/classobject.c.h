#pragma once

static void method_dealloc(PyMethodObject *im);
static PyObject *method_getattro(PyObject *obj, PyObject *name);
static PyObject *method_descr_get(PyObject *meth, PyObject *obj, PyObject *cls);
static PyObject *method_new(PyTypeObject *type, PyObject *args, PyObject *kw);

PyTypeObject PyMethod_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "method",
  .tp_basicsize = sizeof(PyMethodObject),
  .tp_dealloc = (destructor) method_dealloc,
  .tp_vectorcall_offset = offsetof(PyMethodObject, vectorcall),
  .tp_call = PyVectorcall_Call,
  .tp_getattro = method_getattro,
  .tp_setattro = PyObject_GenericSetAttr,
  .tp_flags = Py_TPFLAGS_HAVE_VECTORCALL,
  .tp_descr_get = method_descr_get,
  .tp_new = method_new,
};

static PyObject *
method_vectorcall(PyObject *method, PyObject *const *args,
    size_t nargsf, PyObject *kwnames) {
  assert(Py_IS_TYPE(method, &PyMethod_Type));
  
  PyThreadState *tstate = _PyThreadState_GET();
  PyObject *self = PyMethod_GET_SELF(method);
  PyObject *func = PyMethod_GET_FUNCTION(method);
  Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);

  PyObject *result;
  if (nargs & PY_VECTORCALL_ARGUMENTS_OFFSET) {
    assert(false);
  } else {
    Py_ssize_t nkwargs = (kwnames == NULL) ? 0 : PyTuple_GET_SIZE(kwnames);
    Py_ssize_t totalargs = nargs + nkwargs;
    if (totalargs == 0) {
      return _PyObject_VectorcallTstate(tstate, func, &self, 1, NULL);
    }

    PyObject *newargs_stack[_PY_FASTCALL_SMALL_STACK];
    PyObject **newargs;

    if (totalargs <= (Py_ssize_t) Py_ARRAY_LENGTH(newargs_stack) - 1) {
      newargs = newargs_stack;
    } else {
      newargs = PyMem_Malloc((totalargs + 1) * sizeof(PyObject *));
      if (newargs == NULL) {
        assert(false);
      }
    }
    newargs[0] = self;
    assert(args != NULL);
    memcpy(newargs + 1, args, totalargs * sizeof(PyObject *));
    result = _PyObject_VectorcallTstate(tstate, func,
        newargs, nargs + 1, kwnames);
    if (newargs != newargs_stack) {
      PyMem_Free(newargs);
    }
  }
  return result;
}

PyObject *PyMethod_New(PyObject *func, PyObject *self) {
  if (self == NULL) {
    assert(false);
  }
  PyMethodObject *im = PyObject_GC_New(PyMethodObject, &PyMethod_Type);
  if (im == NULL) {
    return NULL;
  }
  im->im_weakreflist = NULL;
  Py_INCREF(func);
  im->im_func = func;
  Py_INCREF(self);
  im->im_self= self;
  im->vectorcall = method_vectorcall;
  return (PyObject *) im;
}

static void method_dealloc(PyMethodObject *im) {
  if (im->im_weakreflist != NULL) {
    assert(false);
  }
  Py_DECREF(im->im_func);
  Py_XDECREF(im->im_self);
  PyObject_GC_Del(im);
}

static PyObject *method_getattro(PyObject *obj, PyObject *name) {
  assert(false);
}

static PyObject *method_descr_get(PyObject *meth, PyObject *obj, PyObject *cls) {
  assert(false);
}

static PyObject *method_new(PyTypeObject *type, PyObject *args, PyObject *kw) {
  assert(false);
}
