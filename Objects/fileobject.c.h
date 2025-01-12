#pragma once

_Py_IDENTIFIER(open);

PyObject * PyFile_OpenCodeObject(PyObject *path) {
  PyObject *iomod, *f = NULL;

  if (!PyUnicode_Check(path)) {
    fail(0);
  }
  // TODO support hook as cpy
  {
    iomod = PyImport_ImportModule("_io");
    if (iomod) {
      f = _PyObject_CallMethodId(iomod, &PyId_open, "Os",
          path, "rb");
      Py_DECREF(iomod);
    }
  }
  return f;
}
