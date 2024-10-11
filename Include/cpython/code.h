#pragma once

struct PyCodeObject {
	PyObject_HEAD

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
};

#define _Py_OPCODE(word) ((word) & 255)
#define _Py_OPARG(word) ((word) >> 8)

#define PyCode_GetNumFree(op) (PyTuple_GET_SIZE((op)->co_freevars))

