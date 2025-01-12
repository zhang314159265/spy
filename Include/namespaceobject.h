#pragma once

typedef struct {
  PyObject_HEAD
  PyObject *ns_dict;
} _PyNamespaceObject;

static void
namespace_dealloc(_PyNamespaceObject *ns) {
  Py_CLEAR(ns->ns_dict);
  Py_TYPE(ns)->tp_free((PyObject *) ns);
}

static int
namespace_init(_PyNamespaceObject *ns, PyObject *args, PyObject *kwds) {
  fail(0);
}

static PyObject *
namespace_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  PyObject *self;

  assert(type != NULL && type->tp_alloc != NULL);
  self = type->tp_alloc(type, 0);
  if (self != NULL) {
    _PyNamespaceObject *ns = (_PyNamespaceObject *) self;
    ns->ns_dict = PyDict_New();
    if (ns->ns_dict == NULL) {
      fail(0);
    }
  }
  return self;
}

PyTypeObject _PyNamespace_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "types.SimpleNamespace",
  .tp_basicsize = sizeof(_PyNamespaceObject),
  .tp_dealloc = (destructor) namespace_dealloc,
  .tp_getattro = PyObject_GenericGetAttr,
  .tp_setattro = PyObject_GenericSetAttr,
  .tp_dictoffset = offsetof(_PyNamespaceObject, ns_dict),
  .tp_init = (initproc) namespace_init,
  .tp_alloc = PyType_GenericAlloc,
  .tp_new = (newfunc) namespace_new,
  .tp_free = PyObject_GC_Del,
};

PyObject *
_PyNamespace_New(PyObject *kwds) {
  PyObject *ns = namespace_new(&_PyNamespace_Type, NULL, NULL);
  if (ns == NULL)
    return NULL;

  if (kwds == NULL)
    return ns;
  if (PyDict_Update(((_PyNamespaceObject *)ns)->ns_dict, kwds) != 0) {
    Py_DECREF(ns);
    return NULL;
  }
  return (PyObject *) ns;
}
