#pragma once

_Py_IDENTIFIER(__IOBase_closed);

typedef struct {
  PyObject_HEAD

  PyObject *dict;
  PyObject *weakreflist;
} iobase;

static PyObject *
iobase_iter(PyObject *self) {
  fail(0);
}

static PyObject *
iobase_iternext(PyObject *self) {
  fail(0);
}

static int
iobase_clear(iobase *self) {
  fail(0);
}

static void
iobase_dealloc(iobase *self) {
  fail(0);
}

static PyObject *
_io__IOBase_tell_impl(PyObject *self) {
  _Py_IDENTIFIER(seek);
  return _PyObject_CallMethodId(self, &PyId_seek, "ii", 0, 1);
}

static PyObject *
_io__IOBase_tell(PyObject *self, PyObject *ignored) {
  return _io__IOBase_tell_impl(self);
}


#define _IO__IOBASE_TELL_METHODDEF \
  {"tell", (PyCFunction) _io__IOBase_tell, METH_NOARGS, ""},

static PyObject *
iobase_seek(PyObject *self, PyObject *args) {
  fail(0);
}

static int
iobase_check_closed(PyObject *self) {
  fail(0);
}

static PyObject *
iobase_enter(PyObject *self, PyObject *args) {
  #if 0
  if (iobase_check_closed(self))
    return NULL;
  #endif

  Py_INCREF(self);
  return self;
}

static PyObject *
iobase_exit(PyObject *self, PyObject *args) {
  return PyObject_CallMethodNoArgs(self, _PyIO_str_close);
}

static int
iobase_is_closed(PyObject *self) {
  PyObject *res;
  int ret;
  // This gets the derived attribute, which is *not* __IOBase_closed
  // in most cases!
  ret = _PyObject_LookupAttrId(self, &PyId___IOBase_closed, &res);
  Py_XDECREF(res);
  return ret;
}

static PyObject *
_io__IOBase_close_impl(PyObject *self) {
  PyObject *res, *exc, *val, *tb;
  int rc, closed = iobase_is_closed(self);

  if (closed < 0) {
    return NULL;
  }
  if (closed) {
    Py_RETURN_NONE;
  }

  res = PyObject_CallMethodNoArgs(self, _PyIO_str_flush);

  PyErr_Fetch(&exc, &val, &tb);
  rc = _PyObject_SetAttrId(self, &PyId___IOBase_closed, Py_True);
  _PyErr_ChainExceptions(exc, val, tb);
  if (rc < 0) {
    Py_CLEAR(res);
  }

  if (res == NULL)
    return NULL;

  Py_DECREF(res);
  Py_RETURN_NONE;
}

static PyObject *
_io__IOBase_close(PyObject *self, PyObject *ignored) {
  return _io__IOBase_close_impl(self);
}

static int iobase_is_closed(PyObject *self);

static PyObject *
_io__IOBase_flush_impl(PyObject *self) {
  int closed = iobase_is_closed(self);

  if (!closed)
    Py_RETURN_NONE;

  if (closed > 0) {
    fail(0);
  }
  return NULL;
}

static PyObject *
_io__IOBase_flush(PyObject *self, PyObject *ignored) {
  return _io__IOBase_flush_impl(self);
}

#define _IO__IOBASE_CLOSE_METHODDEF \
  {"close", (PyCFunction) _io__IOBase_close, METH_NOARGS, ""},

#define _IO__IOBASE_FLUSH_METHODDEF \
  {"flush", (PyCFunction) _io__IOBase_flush, METH_NOARGS, ""},

static PyMethodDef iobase_methods[] = {
  {"seek", iobase_seek, METH_VARARGS, ""},
  _IO__IOBASE_TELL_METHODDEF
  _IO__IOBASE_CLOSE_METHODDEF
  _IO__IOBASE_FLUSH_METHODDEF

  {"__enter__", iobase_enter, METH_NOARGS},
  {"__exit__", iobase_exit, METH_VARARGS},
  {NULL, NULL},
};

static PyGetSetDef iobase_getset[] = {
  {NULL}
};

static void
iobase_finalize(PyObject *self) {
  fail(0);
}

PyTypeObject PyIOBase_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "_io._IOBase",
  .tp_basicsize = sizeof(iobase),
  .tp_dealloc = (destructor) iobase_dealloc,
  .tp_flags = Py_TPFLAGS_BASETYPE,
  .tp_clear = (inquiry) iobase_clear,
  .tp_weaklistoffset = offsetof(iobase, weakreflist),
  .tp_iter = iobase_iter,
  .tp_iternext = iobase_iternext,
  .tp_methods = iobase_methods,
  .tp_getset = iobase_getset,
  .tp_dictoffset = offsetof(iobase, dict),
  .tp_new = PyType_GenericNew,
  .tp_finalize = iobase_finalize,
};

static PyMethodDef rawiobase_methods[] = {
  {NULL, NULL},
};

PyTypeObject PyRawIOBase_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "_io._RawIOBase",
  .tp_flags = Py_TPFLAGS_BASETYPE,
  .tp_methods = rawiobase_methods,
  .tp_base = &PyIOBase_Type,
};

int _PyIOBase_finalize(PyObject *self) {
  int is_zombie;

  is_zombie = (Py_REFCNT(self) == 0);
  if (is_zombie) {
    return PyObject_CallFinalizerFromDealloc(self);
  } else {
    PyObject_CallFinalizer(self);
    return 0;
  }
  fail(0);
}

PyObject *_PyIOBase_check_readable(PyObject *self, PyObject *args) {
  PyObject *res = PyObject_CallMethodNoArgs(self, _PyIO_str_readable);
  if (res == NULL)
    return NULL;
  if (res != Py_True) {
    Py_CLEAR(res);
    fail(0);
  }
  if (args == Py_True) {
    Py_DECREF(res);
  }
  return res;
}
