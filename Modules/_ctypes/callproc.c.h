#pragma once

static PyObject *py_dl_open(PyObject *self, PyObject *args) {
  PyObject *name, *name2;
  const char *name_str;
  void *handle;
  int mode = RTLD_NOW | RTLD_LOCAL;

  if (!PyArg_ParseTuple(args, "O|i:dlopen", &name, &mode))
    return NULL;

  mode |= RTLD_NOW;
  if (name != Py_None) {
    if (PyUnicode_FSConverter(name, &name2) == 0)
      return NULL;
    name_str = PyBytes_AS_STRING(name2);
  } else {
    fail(0);
  }
  handle = ctypes_dlopen(name_str, mode);
  Py_XDECREF(name2);
  if (!handle) {
    fail(0);
  }
  return PyLong_FromVoidPtr(handle);
}

PyMethodDef _ctypes_module_methods[] = {
  {"dlopen", py_dl_open, METH_VARARGS, ""}, 
  {NULL, NULL},
};
