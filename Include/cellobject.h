#pragma once

typedef struct {
  PyObject_HEAD
  PyObject *ob_ref;
} PyCellObject;

#define PyCell_Check(op) Py_IS_TYPE(op, &PyCell_Type)

PyTypeObject PyCell_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "cell",
  .tp_basicsize = sizeof(PyCellObject),
  .tp_itemsize = 0,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
};
