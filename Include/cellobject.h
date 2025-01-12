#pragma once

typedef struct {
  PyObject_HEAD
  PyObject *ob_ref;
} PyCellObject;

#define PyCell_Check(op) Py_IS_TYPE(op, &PyCell_Type)
#define PyCell_GET(op) (((PyCellObject *)(op))->ob_ref)
#define PyCell_SET(op, v) ((void)(((PyCellObject *)(op))->ob_ref = v))

static void
cell_dealloc(PyCellObject *op) {
  Py_XDECREF(op->ob_ref);
  PyObject_GC_Del(op);
}

PyTypeObject PyCell_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "cell",
  .tp_basicsize = sizeof(PyCellObject),
  .tp_itemsize = 0,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
  .tp_dealloc = (destructor) cell_dealloc,
};

PyObject *PyCell_Get(PyObject *op) {
  if (!PyCell_Check(op)) {
    assert(false);
  }
  Py_XINCREF(((PyCellObject *)op)->ob_ref);
  return PyCell_GET(op);
}

int
PyCell_Set(PyObject *op, PyObject *obj) {
  PyObject *oldobj;
  if (!PyCell_Check(op)) {
    fail(0);
  }
  oldobj = PyCell_GET(op);
  Py_XINCREF(obj);
  PyCell_SET(op, obj);
  Py_XDECREF(oldobj);
  return 0;
}

PyObject *
PyCell_New(PyObject *obj) {
  PyCellObject *op;

  op = (PyCellObject *) PyObject_GC_New(PyCellObject, &PyCell_Type);
  if (op == NULL)
    return NULL;
  op->ob_ref = obj;
  Py_XINCREF(obj);

  _PyObject_GC_TRACK(op);
  return (PyObject *) op;
}
