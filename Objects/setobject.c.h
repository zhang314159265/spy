#pragma once

static int set_init(PySetObject *self, PyObject *args, PyObject *kwds) {
  PyObject *iterable = NULL;

  if (!_PyArg_NoKeywords("set", kwds))
    return -1;
  if (!PyArg_UnpackTuple(args, Py_TYPE(self)->tp_name, 0, 1, &iterable))
    return -1;
  if (self->fill) {
    fail(0);
  }
  self->hash = -1;
  if (iterable == NULL) 
    return 0;
  return set_update_internal(self, iterable);
}


