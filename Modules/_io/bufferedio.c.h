#pragma once

typedef off_t Py_off_t;

#define VALID_READ_BUFFER(self) \
  (self->readable && self->read_end != -1)

#define READAHEAD(self) \
  ((self->readable && VALID_READ_BUFFER(self)) \
    ? (self->read_end - self->pos) : 0)

#define CHECK_INITIALIZED(self) \
  if (self->ok <= 0) { \
    fail(0); \
  }

#define ENTER_BUFFERED(self) \
  ((PyThread_acquire_lock(self->lock, 0) ? \
    1 : _enter_buffered_busy(self)) \
    && (self->owner = PyThread_get_thread_ident(), 1))

#define LEAVE_BUFFERED(self) \
  do { \
    self->owner = 0; \
    PyThread_release_lock(self->lock); \
  } while (0)

static PyMethodDef bufferediobase_methods[] = {
  {NULL, NULL},
};

PyTypeObject PyBufferedIOBase_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "_io.BufferedIOBase",
  .tp_flags = Py_TPFLAGS_BASETYPE,
  .tp_methods = bufferediobase_methods,
  .tp_base = &PyIOBase_Type,
};

Py_off_t PyNumber_AsOff_t(PyObject *item, PyObject *err);

typedef struct {
  PyObject_HEAD

  PyObject *raw;
  int ok;
  int detached;
  int readable;
  int writable;
  char finalizing;

  int fast_closed_checks;
  Py_off_t abs_pos;

  char *buffer;
  Py_off_t pos;
  Py_off_t raw_pos;

  Py_off_t read_end;

  PyThread_type_lock lock;
  volatile unsigned long owner;

  Py_ssize_t buffer_size;
  Py_ssize_t buffer_mask;

  PyObject *dict;
  PyObject *weakreflist;
} buffered;

static int
_enter_buffered_busy(buffered *self) {
  fail(0);
}



PyObject *_PyIOBase_check_readable(PyObject *self, PyObject *args);

static Py_off_t
_buffered_raw_tell(buffered *self) {
  Py_off_t n;
  PyObject *res;
  res = PyObject_CallMethodNoArgs(self->raw, _PyIO_str_tell);
  if (res == NULL)
    return -1;
  n = PyNumber_AsOff_t(res, PyExc_ValueError);
  Py_DECREF(res);
  if (n < 0) {
    fail(0);
  }
  self->abs_pos = n;
  return n;
}

static int
_buffered_init(buffered *self) {
  Py_ssize_t n;
  if (self->buffer_size <= 0) {
    fail(0);
  }
  if (self->buffer)
    PyMem_Free(self->buffer);

  self->buffer = PyMem_Malloc(self->buffer_size);
  if (self->buffer == NULL) {
    fail(0);
  }
  if (self->lock)
    fail(0);
  self->lock = PyThread_allocate_lock();
  if (self->lock == NULL) {
    fail(0);
  }
  self->owner = 0;
  // Find out whether buffer_size is a power of 2
  for (n = self->buffer_size - 1; n & 1; n >>= 1)
    ;
  if (n == 0)
    self->buffer_mask = self->buffer_size - 1;
  else
    self->buffer_mask = 0;
  if (_buffered_raw_tell(self) == -1)
    fail(0);
  return 0;
}

static void _bufferedreader_reset_buf(buffered *self) {
  self->read_end = -1;
}

static int
_io_BufferedReader___init___impl(buffered *self, PyObject *raw, Py_ssize_t buffer_size) {
  self->ok = 0;
  self->detached = 0;

  if (_PyIOBase_check_readable(raw, Py_True) == NULL)
    return -1;

  Py_INCREF(raw);
  Py_XSETREF(self->raw, raw);
  self->buffer_size = buffer_size;
  self->readable = 1;
  self->writable = 0;

  if (_buffered_init(self) < 0)
    return -1;
  _bufferedreader_reset_buf(self);

  self->fast_closed_checks = (Py_IS_TYPE(self, &PyBufferedReader_Type) &&
    Py_IS_TYPE(raw, &PyFileIO_Type));
  self->ok = 1;
  return 0;
}

static int
_io_BufferedReader___init__(PyObject *self, PyObject *args, PyObject *kwargs) {
  int return_value = -1;
  static const char * const _keywords[] = {"raw", "buffer_size", NULL};
  static _PyArg_Parser _parser = {NULL, _keywords, "BufferedReader", 0};
  PyObject *argsbuf[2];
  PyObject *const *fastargs;
  Py_ssize_t nargs = PyTuple_GET_SIZE(args);
  Py_ssize_t noptargs = nargs + (kwargs ? PyDict_GET_SIZE(kwargs) : 0) - 1;
  PyObject *raw;
  Py_ssize_t buffer_size = DEFAULT_BUFFER_SIZE;

  fastargs = _PyArg_UnpackKeywords(_PyTuple_CAST(args)->ob_item, nargs, kwargs, NULL, &_parser, 1, 2, 0, argsbuf);
  if (!fastargs) {
    fail(0);
  }
  raw = fastargs[0];
  if (!noptargs) {
    goto skip_optional_pos;
  }
  {
    Py_ssize_t ival = -1;
    PyObject *iobj = _PyNumber_Index(fastargs[1]);
    if (iobj != NULL) {
      ival = PyLong_AsSsize_t(iobj);
      Py_DECREF(iobj);
    }
    if (ival == -1 && PyErr_Occurred()) {
      fail(0);
    }
    buffer_size = ival;
  }
skip_optional_pos:
  return_value = _io_BufferedReader___init___impl((buffered *) self, raw, buffer_size);

exit:
  return return_value;
}

static PyObject *
buffered_simple_flush(buffered *self, PyObject *args) {
  CHECK_INITIALIZED(self);
  return PyObject_CallMethodNoArgs(self->raw, _PyIO_str_flush);
}

static PyObject *
_bufferedreader_read_all(buffered *self) {
  Py_ssize_t current_size;
  PyObject *res = NULL, *data = NULL, *tmp = NULL, *chunks = NULL, *readall;

  // First copy what we have in the current buffer
  current_size = Py_SAFE_DOWNCAST(READAHEAD(self), Py_off_t, Py_ssize_t);
  if (current_size) {
    fail(0);
  }
  if (self->writable) {
    fail(0);
  }
  _bufferedreader_reset_buf(self);
  if (_PyObject_LookupAttr(self->raw, _PyIO_str_readall, &readall) < 0) {
    fail(0);
  }
  if (readall) {
    tmp = _PyObject_CallNoArg(readall);
    Py_DECREF(readall);
    if (tmp == NULL)
      fail(0);
    if (tmp != Py_None && !PyBytes_Check(tmp)) {
      fail(0);
    }
    if (current_size == 0) {
      res = tmp;
    } else {
      fail(0);
    }
    goto cleanup;
  }

  chunks = PyList_New(0);
  fail(0);

cleanup:
  Py_XINCREF(res);
  Py_XDECREF(data);
  Py_XDECREF(tmp);
  Py_XDECREF(chunks);
  return res;
}

static PyObject *
_io__Buffered_read_impl(buffered *self, Py_ssize_t n) {
  PyObject *res;

  CHECK_INITIALIZED(self);
  if (n < -1) {
    fail(0);
  }

  // CHECK_CLOSED(self, "read of closed file");

  if (n == -1) {
    if (!ENTER_BUFFERED(self))
      return NULL;
    res = _bufferedreader_read_all(self);
  } else {
    fail(0);
  }

  LEAVE_BUFFERED(self);
  return res;
}

static PyObject *
_io__Buffered_read(buffered *self, PyObject *const *args, Py_ssize_t nargs) {
  PyObject *return_value = NULL;
  Py_ssize_t n = -1;
  if (!_PyArg_CheckPositional("read", nargs, 0, 1)) {
    goto exit;
  }
  if (nargs < 1) {
    goto skip_optional;
  }
  if (!_Py_convert_optional_to_ssize_t(args[0], &n)) {
    goto exit;
  }
skip_optional:
  return_value = _io__Buffered_read_impl(self, n);
exit:
  return return_value;
}

#define _IO__BUFFERED_READ_METHODDEF \
  {"read", (PyCFunction)(void(*)(void)) _io__Buffered_read, METH_FASTCALL, ""},

static PyMethodDef bufferedreader_methods[] = {
  {"flush", (PyCFunction) buffered_simple_flush, METH_NOARGS},
  _IO__BUFFERED_READ_METHODDEF
  {NULL, NULL},
};

static PyMemberDef bufferedreader_members[] = {
  {NULL},
};

static PyGetSetDef bufferedreader_getset[] = {
  {NULL}
};

static PyObject *
buffered_iternext(buffered *self) {
  fail(0);
}
 
static void
buffered_dealloc(buffered *self) {
  self->finalizing = 1;
  if (_PyIOBase_finalize((PyObject *) self) < 0)
    return;
  self->ok = 0;
  if (self->weakreflist != NULL)
    fail(0);
  Py_CLEAR(self->raw);
  if (self->buffer) {
    PyMem_Free(self->buffer);
    self->buffer = NULL;
  }
  if (self->lock) {
    PyThread_free_lock(self->lock);
    self->lock = NULL;
  }
  Py_CLEAR(self->dict);
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static int buffered_clear(buffered* self) {
  fail(0);
}

PyTypeObject PyBufferedReader_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "_io.BufferedReader",
  .tp_basicsize = sizeof(buffered),
  .tp_dealloc = (destructor) buffered_dealloc,
  .tp_flags = Py_TPFLAGS_BASETYPE,
  .tp_clear = (inquiry) buffered_clear,
  .tp_weaklistoffset = offsetof(buffered, weakreflist),
  .tp_iternext = (iternextfunc) buffered_iternext,
  .tp_methods = bufferedreader_methods,
  .tp_members = bufferedreader_members,
  .tp_getset = bufferedreader_getset,
  .tp_dictoffset = offsetof(buffered, dict),
  .tp_init = _io_BufferedReader___init__,
  .tp_new = PyType_GenericNew,
};
