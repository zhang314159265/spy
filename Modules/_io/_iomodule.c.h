#pragma once

#include "modsupport.h"
#include "fileobject.h"

// TODO follow cpy
#define PyLong_AsOff_t PyLong_AsLong

PyObject *_PyIO_str_readable = NULL;
PyObject *_PyIO_str_tell = NULL;
PyObject *_PyIO_str_close = NULL;
PyObject *_PyIO_str_flush = NULL;
PyObject *_PyIO_str_readall = NULL;

typedef off_t Py_off_t;

Py_off_t PyNumber_AsOff_t(PyObject *item, PyObject *err) {
  Py_off_t result;
  PyObject *runerr;
  PyObject *value = _PyNumber_Index(item);
  if (value == NULL)
    return -1;

  result = PyLong_AsOff_t(value);
  if (result != -1 || !(runerr = PyErr_Occurred()))
    goto finish;
  fail(0);

finish:
  Py_DECREF(value);
  return result;
}

static int
iomodule_traverse(PyObject *mod, visitproc visit, void *arg) {
  fail(0);
}

static int
iomodule_clear(PyObject *mod) {
  fail(0);
}

static void
iomodule_free(PyObject *mod) {
  fail(0);
}

static PyObject *
_io_open_code_impl(PyObject *module, PyObject *path) {
  // printf("open code path is %s\n", (char *) PyUnicode_DATA(path));
  return PyFile_OpenCodeObject(path);
}

static PyObject *
_io_open_code(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames) {
  PyObject *return_value = NULL;
  static const char *const _keywords[] = {"path", NULL};
  static _PyArg_Parser _parser = {NULL, _keywords, "open_code", 0};
  PyObject *argsbuf[1];
  PyObject *path;

  args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser, 1, 1, 0, argsbuf);
  if (!args) {
    goto exit;
  }
  if (!PyUnicode_Check(args[0])) {
    fail(0);
  }
  if (PyUnicode_READY(args[0]) == -1) {
    goto exit;
  }
  path = args[0];
  return_value = _io_open_code_impl(module, path);

exit:
  return return_value;
}

extern PyTypeObject PyFileIO_Type;
extern PyTypeObject PyRawIOBase_Type;
extern PyTypeObject PyBufferedReader_Type;
extern PyTypeObject PyBufferedIOBase_Type;
extern PyTypeObject PyTextIOWrapper_Type;

static PyObject *
_io_open_impl(PyObject *module, PyObject *file, const char *mode,
    int buffering, const char *encoding, const char *errors,
    const char *newline, int closefd, PyObject *opener) {
  // printf("open file %s\n", (char *) PyUnicode_DATA(file));
  unsigned i;
  int creating = 0, reading = 0, writing = 0, appending = 0, updating = 0;
  int text = 0, binary = 0, universal = 0;
  char rawmode[6], *m;
  int line_buffering, is_number;
  long isatty = 0;
  PyObject *raw, *modeobj = NULL, *buffer, *wrapper, *result = NULL, *path_or_fd = NULL;

  _Py_IDENTIFIER(_blksize);
  _Py_IDENTIFIER(isatty);
  _Py_IDENTIFIER(close);

  is_number = PyNumber_Check(file);
  
  if (is_number) {
    fail(0);
  } else {
    path_or_fd = PyOS_FSPath(file);
    if (path_or_fd == NULL) {
      return NULL;
    }
  }

  if (!is_number &&
      !PyUnicode_Check(path_or_fd) &&
      !PyBytes_Check(path_or_fd)) {
    fail(0);
  }

  // Decode mode
  for (i = 0; i < strlen(mode); i++) {
    char c = mode[i];

    switch (c) {
    case 'r':
      reading = 1;
      break;
    case 'b':
      binary = 1;
      break;
    default:
      fail("mode char %c\n", c);
    }

    // c must not be duplicated
    if (strchr(mode + i + 1, c)) {
      fail(0);
    }
  }

  m = rawmode;
  if (creating) *(m++) = 'x';
  if (reading) *(m++) = 'r';
  if (writing) *(m++) = 'w';
  if (appending) *(m++) = 'a';
  if (updating) *(m++) = '+';
  *m = '\0';

  // Parameters validation
  if (universal) {
    fail(0);
  }

  if (text && binary) {
    fail(0);
  }

  if (creating + reading + writing + appending > 1) {
    fail(0);
  }

  if (binary && encoding != NULL) {
    fail(0);
  }

  if (binary && errors != NULL) {
    fail(0);
  }

  if (binary && newline != NULL) {
    fail(0);
  }

  if (binary && buffering == 1) {
    fail(0);
  }

  // Create the Raw file stream
  {
    PyObject *RawIO_class = (PyObject *)& PyFileIO_Type;
    raw = PyObject_CallFunction(RawIO_class, "OsOO",
      path_or_fd, rawmode,
      closefd ? Py_True : Py_False,
      opener);
  }

  if (raw == NULL)
    goto error;
  result = raw;

  Py_DECREF(path_or_fd);
  path_or_fd = NULL;

  modeobj = PyUnicode_FromString(mode);
  if (modeobj == NULL) {
    goto error;
  }

  // buffering
  if (buffering < 0) {
    #if 0 // TODO handle isatty
    PyObject *res = _PyObject_CallMethodIdNoArgs(raw, &PyId_isatty);
    if (res == NULL)
      goto error;
    isatty = PyLong_AsLong(res);
    Py_DECREF(res);
    #else
    isatty = 0;
    #endif
    if (isatty == -1 && PyErr_Occurred())
      goto error;
  }

  if (buffering == 1 || isatty) {
    fail(0);
  } else {
    line_buffering = 0;
  }

  if (buffering < 0) {
    PyObject *blksize_obj;
    blksize_obj = _PyObject_GetAttrId(raw, &PyId__blksize);
    if (blksize_obj == NULL) {
      fail(0);
    }
    buffering = PyLong_AsLong(blksize_obj);
    Py_DECREF(blksize_obj);
    if (buffering == -1 && PyErr_Occurred()) 
      goto error;
  }
  if (buffering < 0) {
    fail(0);
  }

  // if not buffering, returns the raw file object
  if (buffering == 0) {
    fail(0);
  }

  // wraps into a buffered file
  {
    PyObject *Buffered_class;

    if (updating)
      fail(0);
    else if (creating || writing || appending)
      fail(0);
    else if (reading)
      Buffered_class = (PyObject *) &PyBufferedReader_Type;
    else
      fail(0);

    buffer = PyObject_CallFunction(Buffered_class, "Oi", raw, buffering);
  }

  if (buffer == NULL)
    goto error;
  result = buffer;
  Py_DECREF(raw);

  // if binary, returns the buffered file
  if (binary) {
    Py_DECREF(modeobj);
    return result;
  }

  // wraps into a TextIOWrapper
  wrapper = PyObject_CallFunction((PyObject *) &PyTextIOWrapper_Type,
      "OsssO",
      buffer,
      encoding, errors, newline,
      line_buffering ? Py_True : Py_False);
  if (wrapper == NULL)
    fail(0);
  result = wrapper;
  Py_DECREF(buffer);

  fail(0);

error:
  if (result != NULL) {
    PyObject *exc, *val, *tb, *close_result;
    PyErr_Fetch(&exc, &val, &tb);
    close_result = _PyObject_CallMethodIdNoArgs(result, &PyId_close);
    _PyErr_ChainExceptions(exc, val, tb);
    Py_XDECREF(close_result);
    Py_DECREF(result);
  }
  Py_XDECREF(path_or_fd);
  Py_XDECREF(modeobj);
  return NULL;
}

static PyObject *
_io_open(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames) {
  PyObject *return_value = NULL;
  static const char *const _keywords[] = {"file", "mode", "buffering", "encoding", "errors", "newline", "closefd", "opener", NULL};
  static _PyArg_Parser _parser = {NULL, _keywords, "open", 0};
  PyObject *argsbuf[8];
  Py_ssize_t noptargs = nargs + (kwnames ? PyTuple_GET_SIZE(kwnames) : 0) - 1;
  PyObject *file;
  const char *mode = "r";
  int buffering = -1;
  const char *encoding = NULL;
  const char *errors = NULL;
  const char *newline = NULL;
  int closefd = 1;
  PyObject *opener = Py_None;

  args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser, 1, 8, 0, argsbuf);
  if (!args) {
    fail(0);
  }
  file = args[0];
  if (!noptargs) {
    goto skip_optional_pos;
  }
  if (args[1]) {
    if (!PyUnicode_Check(args[1])) {
      fail(0);
    }
    Py_ssize_t mode_length;
    mode = PyUnicode_AsUTF8AndSize(args[1], &mode_length);
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
  if (args[2]) {
    fail(0);
  }
  if (args[3]) {
    fail(0);
  }
  if (args[4]) {
    fail(0);
  }
  if (args[5]) {
    fail(0);
  }
  if (args[6]) {
    fail(0);
  }
  opener = args[7];
skip_optional_pos:
  return_value = _io_open_impl(module, file, mode, buffering, encoding, errors, newline, closefd, opener);

  return return_value;
}

#define _IO_OPEN_CODE_METHODDEF \
  {"open_code", (PyCFunction)(void(*)(void)) _io_open_code, METH_FASTCALL | METH_KEYWORDS, ""},

#define _IO_OPEN_METHODDEF \
  {"open", (PyCFunction)(void(*)(void)) _io_open, METH_FASTCALL | METH_KEYWORDS, ""},

static PyMethodDef module_methods[] = {
  _IO_OPEN_METHODDEF
  _IO_OPEN_CODE_METHODDEF
  {NULL, NULL},
};

typedef struct {
  int initialized;
} _PyIO_State;

struct PyModuleDef _PyIO_Module = {
  PyModuleDef_HEAD_INIT,
  .m_name = "io",
  .m_doc = "",
  .m_size = sizeof(_PyIO_State),
  .m_methods = module_methods,
  .m_slots = NULL,
  .m_traverse = iomodule_traverse,
  .m_clear = iomodule_clear,
  .m_free = (freefunc) iomodule_free,
};

static inline _PyIO_State*
get_io_state(PyObject *module) {
  void *state = PyModule_GetState(module);
  assert(state != NULL);
  return (_PyIO_State *)state;
}

PyMODINIT_FUNC
PyInit__io(void) {
  PyObject *m = PyModule_Create(&_PyIO_Module);
  _PyIO_State *state = NULL;
  if (m == NULL)
    return NULL;
  state = get_io_state(m);
  state->initialized = 0;

#define ADD_TYPE(type) \
  if (PyModule_AddType(m, type) < 0) { \
    fail(0); \
  }

  // TODO add the initialization

  ADD_TYPE(&PyRawIOBase_Type);

  // BufferedReader
  PyBufferedReader_Type.tp_base = &PyBufferedIOBase_Type;
  ADD_TYPE(&PyBufferedReader_Type);

  /* FileIO */
  PyFileIO_Type.tp_base = &PyRawIOBase_Type;
  ADD_TYPE(&PyFileIO_Type);

  /* Interned strings */
#define ADD_INTERNED(name) \
  if (!_PyIO_str_ ## name && \
    !(_PyIO_str_ ## name = PyUnicode_InternFromString(#name))) \
    fail(0);

  ADD_INTERNED(readable)
  ADD_INTERNED(tell)
  ADD_INTERNED(close)
  ADD_INTERNED(flush)
  ADD_INTERNED(readall)

  state->initialized = 1;
  return m;
}
