#pragma once

#include "classobject.h"

static PyGetSetDef func_getsetlist[] = {
  {"__name__", (getter) func_get_name, (setter) func_set_name},
  {NULL}
};

static PyObject *
func_descr_get(PyObject *func, PyObject *obj, PyObject *type) {
  if (obj == Py_None || obj == NULL) {
    Py_INCREF(func);
    return func;
  }
  return PyMethod_New(func, obj);
}

PyTypeObject PyFunction_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "function",
	.tp_basicsize = sizeof(PyFunctionObject),
  .tp_dealloc = (destructor) func_dealloc,
	.tp_flags = Py_TPFLAGS_HAVE_VECTORCALL | Py_TPFLAGS_METHOD_DESCRIPTOR,
	.tp_call = PyVectorcall_Call,
	.tp_vectorcall_offset = offsetof(PyFunctionObject, vectorcall),
  .tp_getset = func_getsetlist,
  .tp_descr_get = func_descr_get,
};

typedef struct {
  PyObject_HEAD
  PyObject *sm_callable;
  PyObject *sm_dict;
} staticmethod;

typedef struct {
  PyObject_HEAD
  PyObject *cm_callable;
  PyObject *cm_dict;
} classmethod;

static void sm_dealloc(staticmethod *sm);
static int sm_init(PyObject *self, PyObject *args, PyObject *kwds);
static PyObject *sm_descr_get(PyObject *self, PyObject *obj, PyObject *type);

PyTypeObject PyStaticMethod_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "staticmethod",
  .tp_basicsize = sizeof(staticmethod),
  .tp_dealloc = (destructor) sm_dealloc,
  .tp_descr_get = sm_descr_get,
  .tp_dictoffset = offsetof(staticmethod, sm_dict),
  .tp_init = sm_init,
  .tp_alloc = PyType_GenericAlloc,
  .tp_new = PyType_GenericNew,
  .tp_free = PyObject_GC_Del,
};

static void cm_dealloc(classmethod *cm);
static PyObject *cm_descr_get(PyObject *self, PyObject *obj, PyObject *type);
static int cm_init(PyObject *self, PyObject *args, PyObject *kwds);

PyTypeObject PyClassMethod_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "classmethod",
  .tp_basicsize = sizeof(classmethod),
  .tp_dealloc = (destructor) cm_dealloc,
  .tp_descr_get = cm_descr_get,
  .tp_dictoffset = offsetof(classmethod, cm_dict),
  .tp_init = cm_init,
  .tp_alloc = PyType_GenericAlloc,
  .tp_new = PyType_GenericNew,
  .tp_free = PyObject_GC_Del,
};

static void sm_dealloc(staticmethod *sm) {
  assert(false);
}

void PyErr_Clear(void);

static int
functools_copy_attr(PyObject *wrapper, PyObject *wrapped, PyObject *name) {
  PyObject *value = PyObject_GetAttr(wrapped, name);
  if (value == NULL) {
    if (PyErr_ExceptionMatches(PyExc_AttributeError)) {
      PyErr_Clear();
      return 0;
    }
    return -1;
  }

  int res = PyObject_SetAttr(wrapper, name, value);
  Py_DECREF(value);
  return res;
}

static int
functools_wraps(PyObject *wrapper, PyObject *wrapped) {
#define COPY_ATTR(ATTR) \
  do { \
    _Py_IDENTIFIER(ATTR); \
    PyObject *attr = _PyUnicode_FromId(&PyId_ ## ATTR); \
    if (attr == NULL) { \
      return -1; \
    } \
    if (functools_copy_attr(wrapper, wrapped, attr) < 0) { \
      return -1; \
    } \
  } while (0)

  COPY_ATTR(__module__);
  COPY_ATTR(__name__);
  COPY_ATTR(__qualname__);
  COPY_ATTR(__doc__);
  COPY_ATTR(__annotations__);
  return 0;

#undef COPY_ATTR
}

static int sm_init(PyObject *self, PyObject *args, PyObject *kwds) {
  staticmethod *sm = (staticmethod *) self;
  PyObject *callable;

  if (!_PyArg_NoKeywords("staticmethod", kwds))
    return -1;
  if (!PyArg_UnpackTuple(args, "staticmethod", 1, 1, &callable))
    return -1;
  Py_INCREF(callable);
  Py_XSETREF(sm->sm_callable, callable);

  if (functools_wraps((PyObject *) sm, sm->sm_callable) < 0) {
    return -1;
  }
  return 0;
}

static PyObject *sm_descr_get(PyObject *self, PyObject *obj, PyObject *type) {
  staticmethod *sm = (staticmethod *) self;

  if (sm->sm_callable == NULL) {
    assert(false);
  }
  Py_INCREF(sm->sm_callable);
  return sm->sm_callable;
}

static void cm_dealloc(classmethod *cm) {
  assert(false);
}

static PyObject *cm_descr_get(PyObject *self, PyObject *obj, PyObject *type) {
  classmethod *cm = (classmethod *) self;

  if (cm->cm_callable == NULL) {
    assert(false);
  }
  if (type == NULL) {
    assert(false);
  }
  if (Py_TYPE(cm->cm_callable)->tp_descr_get != NULL) {
    assert(false);
  }
  return PyMethod_New(cm->cm_callable, type);
}

static int cm_init(PyObject *self, PyObject *args, PyObject *kwds) {
  classmethod *cm = (classmethod *) self;
  PyObject *callable;

  if (!_PyArg_NoKeywords("classmethod", kwds))
    return -1;
  if (!PyArg_UnpackTuple(args, "classmethod", 1, 1, &callable))
    return -1;
  Py_INCREF(callable);
  Py_XSETREF(cm->cm_callable, callable);

  if (functools_wraps((PyObject *) cm, cm->cm_callable) < 0) {
    return -1;
  }
  return 0;
}
