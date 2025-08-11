#pragma once

StgDictObject *
PyType_stgdict(PyObject *obj) {
  PyTypeObject *type;

  if (!PyType_Check(obj)) {
    return NULL;
  }
  type = (PyTypeObject *) obj;
  if (!type->tp_dict || !PyCStgDict_CheckExact(type->tp_dict))
    return NULL;

  return (StgDictObject *) type->tp_dict;
}

StgDictObject *
PyObject_stgdict(PyObject *self) {
  PyTypeObject *type = Py_TYPE(self);
  if (!type->tp_dict || !PyCStgDict_CheckExact(type->tp_dict))
    return NULL;
  return (StgDictObject *) type->tp_dict;
}

static void
PyCStgDict_dealloc(StgDictObject *self) {
  fail(0);
}

static struct PyMethodDef PyCStgDict_methods[] = {
  {NULL, NULL},
};

static int
PyCStgDict_init(StgDictObject *self, PyObject *args, PyObject *kwds) {
  if (PyDict_Type.tp_init((PyObject *) self, args, kwds) < 0)
    return -1;

  self->format = NULL;
  self->ndim = 0;
  self->shape = NULL;
  return 0;
}

PyTypeObject PyCStgDict_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "StgDict",
  .tp_basicsize = sizeof(StgDictObject),
  .tp_dealloc = (destructor) PyCStgDict_dealloc,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_methods = PyCStgDict_methods,
  .tp_init = (initproc) PyCStgDict_init,

};
