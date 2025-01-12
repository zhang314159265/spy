#pragma once

typedef off_t Py_off_t;

#include <fcntl.h>
#include "fileutils.h"

#define DEFAULT_BUFFER_SIZE (8 * 1024) /* bytes */

_Py_IDENTIFIER(name);


typedef struct {
  PyObject_HEAD
  int fd;
  unsigned int created : 1;
  unsigned int readable : 1;
  unsigned int writable : 1;
  unsigned appending : 1;
  signed int seekable : 2; // -1 means unknown
  unsigned int closefd : 1;
  char finalizing;
  unsigned int blksize;
  PyObject *weakreflist;
  PyObject *dict;
} fileio;

int _PyIOBase_finalize(PyObject *self);

static void
fileio_dealloc(fileio *self) {
  self->finalizing = 1;
  if (_PyIOBase_finalize((PyObject *) self) < 0)
    return;
  if (self->weakreflist != NULL)
    fail(0);
  Py_CLEAR(self->dict);
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static int
fileio_clear(fileio *self) {
  fail(0);
}

static PyObject *
fileio_repr(fileio *self) {
  fail(0);
}

static PyObject *
_io_FileIO_close_impl(fileio *self) {
  fail(0);
}

static PyObject *
_io_FileIO_close(fileio *self, PyObject *ignored) {
  return _io_FileIO_close_impl(self);
}

#define _IO_FILEIO_CLOSE_METHODDEF \
  {"close", (PyCFunction) _io_FileIO_close, METH_NOARGS, ""},

static PyObject *
_io_FileIO_readable_impl(fileio *self) {
  if (self->fd < 0) {
    fail(0);
  }
  return PyBool_FromLong((long) self->readable);
}

static PyObject *
_io_FileIO_readable(fileio *self, PyObject *ignored) {
  return _io_FileIO_readable_impl(self);
}

static PyObject *
_io_FileIO_readall_impl(fileio *self) {
  struct _Py_stat_struct status;
  Py_off_t pos, end;
  PyObject *result;
  Py_ssize_t bytes_read = 0;
  Py_ssize_t n;
  size_t bufsize;
  int fstat_result;

  if (self->fd < 0)
    fail(0);

  pos = lseek(self->fd, 0L, SEEK_CUR);
  fstat_result = _Py_fstat_noraise(self->fd, &status);

  if (fstat_result == 0)
    end = status.st_size;
  else
    end = (Py_off_t) -1;

  if (end > 0 && end >= pos && pos >= 0 && end - pos < PY_SSIZE_T_MAX) {
    bufsize = (size_t)(end - pos + 1);
  } else {
    fail(0);
  }

  result = PyBytes_FromStringAndSize(NULL, bufsize);
  if (result == NULL)
    return NULL;

  while (1) {
    if (bytes_read >= (Py_ssize_t) bufsize) {
      fail(0);
    }

    n = _Py_read(self->fd,
        PyBytes_AS_STRING(result) + bytes_read,
        bufsize - bytes_read);
    if (n == 0)
      break;
    if (n == -1) {
      fail(0);
    }
    bytes_read += n;
    pos += n;
  }

  if (PyBytes_GET_SIZE(result) > bytes_read) {
    if (_PyBytes_Resize(&result, bytes_read) < 0)
      return NULL;
  }
  return result;
}

static PyObject *
_io_FileIO_readall(fileio *self, PyObject *ignored) {
  return _io_FileIO_readall_impl(self);
}


#define _IO_FILEIO_READABLE_METHODDEF \
  {"readable", (PyCFunction) _io_FileIO_readable, METH_NOARGS, ""},

#define _IO_FILEIO_READALL_METHODDEF \
  {"readall", (PyCFunction) _io_FileIO_readall, METH_NOARGS, ""},

#define _IO_FILEIO_SEEK_METHODDEF \
  {"seek", (PyCFunction)(void(*)(void)) _io_FileIO_seek, METH_FASTCALL, ""},

static PyObject *
portable_lseek(fileio *self, PyObject *posobj, int whence, bool supress_pipe_error) {
  Py_off_t pos, res;
  int fd = self->fd;

  switch (whence) {
  case 0: whence = SEEK_SET; break;
  case 1: whence = SEEK_CUR; break;
  case 2: whence = SEEK_END; break;
  }

  if (posobj == NULL) {
    pos = 0;
  } else {
    pos = PyLong_AsLong(posobj);
    if (PyErr_Occurred())
      return NULL;
  }
  res = lseek(fd, pos, whence);
  if (self->seekable < 0) {
    self->seekable = (res >= 0);
  }
  if (res < 0) {
    fail(0);
  }
  return PyLong_FromLong(res);
}


static PyObject *
_io_FileIO_seek_impl(fileio *self, PyObject *pos, int whence) {
  if (self->fd < 0)
    fail(0);

  return portable_lseek(self, pos, whence, false);
}

static PyObject *
_io_FileIO_seek(fileio *self, PyObject *const *args, Py_ssize_t nargs) {
  PyObject *return_value = NULL;
  PyObject *pos;
  int whence = 0;

  if (!_PyArg_CheckPositional("seek", nargs, 1, 2)) {
    fail(0);
  }
  pos = args[0];
  if (nargs < 2) {
    goto skip_optional;
  }
  whence = _PyLong_AsInt(args[1]);
  if (whence == -1 && PyErr_Occurred()) {
    fail(0);
  }
skip_optional:
  return_value = _io_FileIO_seek_impl(self, pos, whence);

exit:
  return return_value;
}

static PyMethodDef fileio_methods[] = {
  _IO_FILEIO_CLOSE_METHODDEF
  _IO_FILEIO_READABLE_METHODDEF
  _IO_FILEIO_READALL_METHODDEF
  _IO_FILEIO_SEEK_METHODDEF
  {NULL, NULL},
};

static PyMemberDef fileio_members[] = {
  {"_blksize", T_UINT, offsetof(fileio, blksize), 0},
  {NULL},
};

static PyGetSetDef fileio_getsetlist[] = {
  {NULL},
};

static PyObject *
fileio_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  fileio *self;

  assert(type != NULL && type->tp_alloc != NULL);

  self = (fileio *) type->tp_alloc(type, 0);
  if (self != NULL) {
    self->fd = -1;
    self->created = 0;
    self->readable = 0;
    self->writable = 0;
    self->appending = 0;
    self->seekable = -1;
    self->blksize = 0;
    self->closefd = 1;
    self->weakreflist = NULL;
  }

  return (PyObject *) self;
}

#define PyFileIO_Check(op) (PyObject_TypeCheck((op), &PyFileIO_Type))

static int
_io_FileIO___init___impl(fileio *self, PyObject *nameobj, const char *mode, int closefd, PyObject *opener) {
  const char *name = NULL;
  PyObject *stringobj = NULL;
  const char *s;
  int ret = 0;
  int rwa = 0;
  int flags = 0;
  int fd = -1;
  int fd_is_own = 0;
  struct _Py_stat_struct fdfstat;
  int fstat_result;

  assert(PyFileIO_Check(self));
  if (self->fd >= 0) {
    fail(0);
  }

  fd = _PyLong_AsInt(nameobj);
  if (fd < 0) {
    if (!PyErr_Occurred()) {
      fail(0);
    }
    PyErr_Clear();
  }

  if (fd < 0) {
    if (!PyUnicode_FSConverter(nameobj, &stringobj)) {
      return -1;
    }
    name = PyBytes_AS_STRING(stringobj);
  }

  s = mode;
  while (*s) {
    switch (*s++) {
    case 'x': fail("x");
    case 'r': 
      if (rwa)
        fail(0);
      rwa = 1;
      self->readable = 1;
      break;
    case 'w': fail("w");
    case 'a': fail("a");
    case 'b': fail("b");
    case '+': fail("+");
    default:
      fail(0);
    }
  }

  if (!rwa)
    fail(0);

  if (self->readable && self->writable) {
    fail(0);
  } else if (self->readable)
    flags |= O_RDONLY;
  else
    flags |= O_WRONLY;

#ifdef O_BINARY
  flags |= O_BINARY;
#endif

#ifdef O_CLOEXEC
  flags |= O_CLOEXEC;
#endif

  if (fd >= 0) {
    fail(0);
  } else {
    self->closefd = 1;
    if (!closefd) {
      fail(0);
    }
    errno = 0;
    if (opener == Py_None) {
      do {
        self->fd = open(name, flags, 0666);
      } while (self->fd < 0 && errno == EINTR);
    } else {
      fail(0);
    }

    fd_is_own = 1;
    if (self->fd < 0) {
      printf("fail to open %s with flags %d\n", name, flags);
      PyErr_SetFromErrnoWithFilenameObject(PyExc_OSError, nameobj);
      goto error;
    }
    // TODO set inheritable
  }

  self->blksize = DEFAULT_BUFFER_SIZE;
  fstat_result = _Py_fstat_noraise(self->fd, &fdfstat);
  if (fstat_result < 0) {
    fail(0);
  } else {
    if (S_ISDIR(fdfstat.st_mode)) {
      errno = EISDIR;
      fail(0);
    }
    if (fdfstat.st_blksize > 1)
      self->blksize = fdfstat.st_blksize;
  }

  if (_PyObject_SetAttrId((PyObject *) self, &PyId_name, nameobj) < 0)
    goto error;

  if (self->appending) {
    fail(0);
  }

  goto done;

error:
  ret = -1;
  if (!fd_is_own)
    self->fd = -1;
  if (self->fd >= 0) {
    fail(0);
  }
done:
  Py_CLEAR(stringobj);
  return ret;
}

static int
_io_FileIO___init__(PyObject *self, PyObject *args, PyObject *kwargs) {
  int return_value = -1;
  static const char *const _keywords[] = {"file", "mode", "closefd", "opener", NULL};
  static _PyArg_Parser _parser = {NULL, _keywords, "FileIO", 0};
  PyObject *argsbuf[4];
  PyObject *const *fastargs;
  Py_ssize_t nargs = PyTuple_GET_SIZE(args);
  Py_ssize_t noptargs = nargs + (kwargs ? PyDict_GET_SIZE(kwargs) : 0) - 1;
  PyObject *nameobj;
  const char *mode = "r";
  int closefd = 1;
  PyObject *opener = Py_None;

  fastargs = _PyArg_UnpackKeywords(_PyTuple_CAST(args)->ob_item, nargs, kwargs, NULL, &_parser, 1, 4, 0, argsbuf);
  if (!fastargs) {
    fail(0);
  }
  nameobj = fastargs[0];
  if (!noptargs) {
    goto skip_optional_pos;
  }
  if (fastargs[1]) {
    if (!PyUnicode_Check(fastargs[1])) {
      fail(0);
    }
    Py_ssize_t mode_length;
    mode = PyUnicode_AsUTF8AndSize(fastargs[1], &mode_length);
    if (mode == NULL) {
      fail(0);
    }
    if (strlen(mode) != (size_t) mode_length) {
      fail(0);
    }
    if (!--noptargs) {
      goto skip_optional_pos;
    }
  }
  if (fastargs[2]) {
    closefd = _PyLong_AsInt(fastargs[2]);
    if (closefd == -1 && PyErr_Occurred()) {
      fail(0);
    }
    if (!--noptargs) {
      goto skip_optional_pos;
    }
  }
  opener = fastargs[3];
skip_optional_pos:
  return_value = _io_FileIO___init___impl((fileio *) self, nameobj, mode, closefd, opener);

exit:
  return return_value;
}

PyTypeObject PyFileIO_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "_io.FileIO",
  .tp_basicsize = sizeof(fileio),
  .tp_dealloc = (destructor) fileio_dealloc,
  .tp_repr = (reprfunc) fileio_repr,
  .tp_getattro = PyObject_GenericGetAttr,
  .tp_flags = Py_TPFLAGS_BASETYPE,
  .tp_clear = (inquiry) fileio_clear,
  .tp_weaklistoffset = offsetof(fileio, weakreflist),
  .tp_methods = fileio_methods,
  .tp_members = fileio_members,
  .tp_getset = fileio_getsetlist,
  .tp_dictoffset = offsetof(fileio, dict),
  .tp_init = _io_FileIO___init__,
  .tp_alloc = PyType_GenericAlloc,
  .tp_new = fileio_new,
  .tp_free = PyObject_GC_Del,
};
