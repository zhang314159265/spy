// #pragma once

PyObject *STRINGLIB(rpartition)(PyObject *str_obj,
    const STRINGLIB_CHAR *str, Py_ssize_t str_len,
    PyObject *sep_obj,
    const STRINGLIB_CHAR *sep, Py_ssize_t sep_len) {
  PyObject *out;
  Py_ssize_t pos;

  if (sep_len == 0) {
    fail(0);
  }

  out = PyTuple_New(3);
  if (!out) {
    return NULL;
  }

  pos = FASTSEARCH(str, str_len, sep, sep_len, -1, FAST_RSEARCH);

  if (pos < 0) {
#if STRINGLIB_MUTABLE
    fail(0);
#else
    PyObject *empty = (PyObject*) STRINGLIB_GET_EMPTY();
    assert(empty != NULL);
    Py_INCREF(empty);
    PyTuple_SET_ITEM(out, 0, empty);
    Py_INCREF(empty);
    PyTuple_SET_ITEM(out, 1, empty);
    Py_INCREF(str_obj);
    PyTuple_SET_ITEM(out, 2, (PyObject *) str_obj);
#endif
    return out;
  }

  PyTuple_SET_ITEM(out, 0, STRINGLIB_NEW(str, pos));
  Py_INCREF(sep_obj);
  PyTuple_SET_ITEM(out, 1, sep_obj);
  pos += sep_len;
  PyTuple_SET_ITEM(out, 2, STRINGLIB_NEW(str + pos, str_len - pos));

  if (PyErr_Occurred()) {
    fail(0);
  }

  return out;
}
