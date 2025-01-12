#pragma once

typedef struct {
  PyObject_HEAD
  int ok;
  int detached;

  PyObject *weakreflist;
  PyObject *dict;
} textio;

static void
textiowrapper_dealloc(textio *self) {
  fail(0);
}

static int
textiowrapper_clear(textio *self) {
  fail(0);
}

static PyObject *
textiowrapper_iternext(textio *self) {
  fail(0);
}

static PyMethodDef textiowrapper_methods[] = {
  {NULL, NULL},
};

static PyMemberDef textiowrapper_members[] = {
  {NULL},
};

static PyGetSetDef textiowrapper_getset[] = {
  {NULL},
};

static int
_io_TextIOWrapper___init__(PyObject *self, PyObject *args, PyObject *kwargs) {
  fail(0);
}

PyTypeObject PyTextIOWrapper_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = ".io.TextIOWrapper",
  .tp_basicsize = sizeof(textio),
  .tp_dealloc = (destructor) textiowrapper_dealloc,
  .tp_flags = Py_TPFLAGS_BASETYPE,
  .tp_clear = (inquiry) textiowrapper_clear,
  .tp_weaklistoffset = offsetof(textio, weakreflist),
  .tp_iternext = (iternextfunc) textiowrapper_iternext,
  .tp_methods = textiowrapper_methods,
  .tp_members = textiowrapper_members,
  .tp_getset = textiowrapper_getset,
  .tp_dictoffset = offsetof(textio, dict),
  .tp_init = _io_TextIOWrapper___init__,
  .tp_new = PyType_GenericNew,
};
