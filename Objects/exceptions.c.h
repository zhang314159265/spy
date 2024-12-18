#pragma once

#include "structmember.h"

static PyObject *
BaseException_str(PyBaseExceptionObject *self) {
  assert(false);
}

static PyMemberDef AttributeError_members[] = {
  {NULL},
};

static PyMethodDef AttributeError_methods[] = {
  {NULL},
};

#define ComplexExtendsException(EXCBASE, EXCNAME, EXCSTORE, EXCNEW, \
    EXCMETHODS, EXCMEMBERS, EXCGETSET, \
    EXCSTR, EXCDOC) \
static PyTypeObject _PyExc_ ## EXCNAME = { \
  PyVarObject_HEAD_INIT(NULL, 0) \
  .tp_name = # EXCNAME, \
  .tp_basicsize = sizeof(Py ## EXCSTORE ## Object), \
  .tp_dealloc = (destructor) EXCSTORE ## _dealloc, \
  .tp_str = (reprfunc) EXCSTR, \
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC, \
  .tp_doc = PyDoc_STR(EXCDOC), \
  .tp_traverse = (traverseproc) EXCSTORE ## _traverse, \
  .tp_clear = (inquiry) EXCSTORE ## _clear, \
  .tp_methods = EXCMETHODS, \
  .tp_members = EXCMEMBERS, \
  .tp_getset = EXCGETSET, \
  .tp_base = &_ ## EXCBASE, \
  .tp_dictoffset = offsetof(Py ## EXCSTORE ## Object, dict), \
  .tp_init = (initproc) EXCSTORE ## _init, \
  .tp_new = EXCNEW, \
}; \
PyObject *PyExc_ ## EXCNAME = (PyObject *) &_PyExc_ ## EXCNAME

#define SimpleExtendsException(EXCBASE, EXCNAME, EXCDOC) \
static PyTypeObject _PyExc_ ## EXCNAME = { \
  PyVarObject_HEAD_INIT(NULL, 0) \
  .tp_name = # EXCNAME, \
  .tp_basicsize = sizeof(PyBaseExceptionObject), \
  .tp_dealloc = (destructor) BaseException_dealloc, \
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC, \
  .tp_doc = PyDoc_STR(EXCDOC), \
  .tp_traverse = (traverseproc) BaseException_traverse, \
  .tp_clear = (inquiry) BaseException_clear, \
  .tp_base = &_ ## EXCBASE, \
  .tp_dictoffset = offsetof(PyBaseExceptionObject, dict), \
  .tp_init = (initproc) BaseException_init, \
  .tp_new = BaseException_new, \
}; \
PyObject *PyExc_ ## EXCNAME = (PyObject *) &_PyExc_ ## EXCNAME

static int
BaseException_traverse(PyBaseExceptionObject *self, visitproc visit, void *arg) {
  assert(false);
}

static int
BaseException_clear(PyBaseExceptionObject *self) {
  Py_CLEAR(self->dict);
  Py_CLEAR(self->args);
  Py_CLEAR(self->traceback);
  Py_CLEAR(self->cause);
  Py_CLEAR(self->context);
  return 0;
}

static void BaseException_dealloc(PyBaseExceptionObject *self) {
  assert(false);
}

static int BaseException_init(PyBaseExceptionObject *self, PyObject *args, PyObject *kwds) {
  if (!_PyArg_NoKeywords(Py_TYPE(self)->tp_name, kwds))
    return -1;

  Py_INCREF(args);
  Py_XSETREF(self->args, args);

  return 0;
}

static PyObject *
BaseException_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  PyBaseExceptionObject *self;

  self = (PyBaseExceptionObject *) type->tp_alloc(type, 0);
  if (!self)
    return NULL;
  self->dict = NULL;
  self->traceback = self->cause = self->context = NULL;
  self->suppress_context = 0;

  if (args) {
    self->args = args;
    Py_INCREF(args);
    return (PyObject *) self;
  }

  self->args = PyTuple_New(0);
  if (!self->args) {
    Py_DECREF(self);
    return NULL;
  }
  return (PyObject *) self;
}

static PyTypeObject _PyExc_BaseException = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "BaseException",
  .tp_basicsize = sizeof(PyBaseExceptionObject),
  .tp_dealloc = (destructor) BaseException_dealloc,
  .tp_getattro = PyObject_GenericGetAttr,
  .tp_setattro = PyObject_GenericSetAttr,
  .tp_dictoffset = offsetof(PyBaseExceptionObject, dict),
  .tp_init = (initproc) BaseException_init,
  .tp_new = BaseException_new,
  .tp_flags = Py_TPFLAGS_BASE_EXC_SUBCLASS,
};

PyObject *PyExc_BaseException = (PyObject *) &_PyExc_BaseException;

SimpleExtendsException(PyExc_BaseException, Exception,
    "Common base class for all non-exit exceptions.");

static int AttributeError_init(PyAttributeErrorObject *self, PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {"name", "obj", NULL};
  PyObject *name = NULL;
  PyObject *obj = NULL;

  if (BaseException_init((PyBaseExceptionObject *) self, args, NULL) == -1) {
    return -1;
  }

  PyObject *empty_tuple = PyTuple_New(0);
  if (!empty_tuple) {
    return -1;
  }
  if (!PyArg_ParseTupleAndKeywords(empty_tuple, kwds, "|$OO:AttributeError", kwlist, &name, &obj)) {
    Py_DECREF(empty_tuple);
    return -1;
  }
  Py_DECREF(empty_tuple);

  Py_XINCREF(name);
  Py_XSETREF(self->name, name);

  Py_XINCREF(obj);
  Py_XSETREF(self->obj, obj);

  return 0;
}

static int AttributeError_clear(PyAttributeErrorObject *self) {
  Py_CLEAR(self->obj);
  Py_CLEAR(self->name);
  return BaseException_clear((PyBaseExceptionObject *) self);
}

static void AttributeError_dealloc(PyAttributeErrorObject *self) {
  AttributeError_clear(self);
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static int AttributeError_traverse(PyAttributeErrorObject *self, visitproc visit, void *arg) {
  assert(false);
}

ComplexExtendsException(PyExc_Exception, AttributeError,
    AttributeError, 0,
    AttributeError_methods, AttributeError_members,
    0, BaseException_str, "Attribute not found.");


PyStatus _PyExc_Init(PyInterpreterState *interp) {
#define PRE_INIT(TYPE) \
  if (!(_PyExc_ ## TYPE.tp_flags & Py_TPFLAGS_READY)) { \
    if (PyType_Ready(&_PyExc_ ## TYPE) < 0) { \
      assert(false); \
    } \
    Py_INCREF(PyExc_ ## TYPE); \
  }
  PRE_INIT(AttributeError);
  return _PyStatus_OK();
}
