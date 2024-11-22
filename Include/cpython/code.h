#pragma once

#define CO_GENERATOR 0x0020
#define CO_COROUTINE 0x0080
#define CO_ASYNC_GENERATOR 0x0200

struct PyCodeObject {
	PyObject_HEAD

  int co_argcount;
  int co_posonlyargcount;
  int co_kwonlyargcount;

  int co_nlocals; // #local variables
  int co_stacksize;
  int co_flags;
  int co_firstlineno;
  PyObject *co_code;
  PyObject *co_consts;
  PyObject *co_names;
  PyObject *co_varnames;

  PyObject *co_name;
  PyObject *co_linetable;
  void *co_extra;

  PyObject *co_freevars;
  PyObject *co_cellvars;
  Py_ssize_t *co_cell2arg;
};

#define _Py_OPCODE(word) ((word) & 255)
#define _Py_OPARG(word) ((word) >> 8)

#define PyCode_GetNumFree(op) (PyTuple_GET_SIZE((op)->co_freevars))

