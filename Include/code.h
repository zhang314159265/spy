#pragma once

#include <math.h>
#include <stdbool.h>

#define CO_CELL_NOT_AN_ARG (-1)

typedef struct PyCodeObject PyCodeObject;

#include "cpython/code.h"
#include "tupleobject.h"
#include "internal/pycore_tuple.h"
#include "cpython/pyctype.h"

#define PyCode_Check(op) Py_IS_TYPE(op, &PyCode_Type)

#define CO_OPTIMIZED 0x0001
#define CO_NEWLOCALS 0x0002
#define CO_VARARGS 0x0004
#define CO_VARKEYWORDS 0x0008
#define CO_NOFREE 0x0040
#define CO_ASYNC_GENERATOR 0x0200

#define CO_MAXBLOCKS 20

typedef uint16_t _Py_CODEUNIT;

static void code_dealloc(PyCodeObject *co);
static Py_hash_t code_hash(PyCodeObject *co);

PyTypeObject PyCode_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "code",
  .tp_basicsize = sizeof(PyCodeObject),
  .tp_dealloc = (destructor) code_dealloc,
  .tp_hash = (hashfunc) code_hash,
};

extern PyTypeObject PyLong_Type;
#define PyLong_CheckExact(op) Py_IS_TYPE(op, &PyLong_Type)

// defined in cpy/Objects/codeobject.c
PyObject *_PyCode_ConstantKey(PyObject *op);

static int
intern_strings(PyObject *tuple)
{
  Py_ssize_t i;
  
  for (i = PyTuple_GET_SIZE(tuple); --i >= 0; ) {
    PyObject *v = PyTuple_GET_ITEM(tuple, i);
    if (v == NULL || !PyUnicode_CheckExact(v)) {
      assert(false);
    }
    PyUnicode_InternInPlace(&_PyTuple_ITEMS(tuple)[i]);
  }
  return 0;
}

static int
all_name_chars(PyObject *o) {
  const unsigned char *s, *e;

  if (!PyUnicode_IS_ASCII(o))
    return 0;

  s = PyUnicode_1BYTE_DATA(o);
  e = s + PyUnicode_GET_LENGTH(o);
  for (; s != e; s++) {
    if (!Py_ISALNUM(*s) && *s != '_')
      return 0;
  }
  return 1;
}

static int
intern_string_constants(PyObject *tuple, int *modified) {
  for (Py_ssize_t i = PyTuple_GET_SIZE(tuple); --i >= 0; ) {
    PyObject *v = PyTuple_GET_ITEM(tuple, i);
    if (PyUnicode_CheckExact(v)) {
      if (PyUnicode_READY(v) == -1) {
        return -1;
      }

      if (all_name_chars(v)) {
        PyObject *w = v;
        PyUnicode_InternInPlace(&v);
        if (w != v) {
          PyTuple_SET_ITEM(tuple, i, v);
          if (modified) {
            *modified = 1;
          }
        }
      }
    } else if (PyTuple_CheckExact(v)) {
      if (intern_string_constants(v, NULL) < 0) {
        return -1;
      }
    }
  }
  return 0;
}

PyCodeObject *
PyCode_NewWithPosOnlyArgs(int argcount,
    int posonlyargcount, int kwonlyargcount,
    int nlocals, int stacksize, int flags,
    PyObject *code, PyObject *consts,
    PyObject *names, PyObject *varnames,
    PyObject *freevars, PyObject *cellvars,
    PyObject *name, int firstlineno,
    PyObject *linetable) {
  PyCodeObject *co;
  Py_ssize_t *cell2arg = NULL;
  Py_ssize_t i, n_cellvars, n_varnames, total_args;

  if (freevars == NULL || !PyTuple_Check(freevars)) {
    assert(false);
  }

  // Ensure that strings are ready Unicode string
  if (PyUnicode_READY(name) < 0) {
    assert(false);
  }

  if (intern_strings(names) < 0) {
    return NULL;
  }
  if (intern_strings(varnames) < 0) {
    return NULL;
  }
  if (intern_strings(freevars) < 0) {
    return NULL;
  }

  if (intern_strings(cellvars) < 0) {
    return NULL;
  }

  if (intern_string_constants(consts, NULL) < 0) {
    return NULL;
  }

  if (PyBytes_GET_SIZE(code) > INT_MAX) {
    assert(false);
  }

  // check for any inner or outer closure references
  n_cellvars = PyTuple_GET_SIZE(cellvars);
  if (!n_cellvars && !PyTuple_GET_SIZE(freevars)) {
    flags |= CO_NOFREE;
  } else {
    flags &= ~CO_NOFREE;
  }
  
  n_varnames = PyTuple_GET_SIZE(varnames);
  if (argcount <= n_varnames && kwonlyargcount <= n_varnames) {
    total_args = (Py_ssize_t) argcount + (Py_ssize_t) kwonlyargcount +
        ((flags & CO_VARARGS) != 0) + ((flags & CO_VARKEYWORDS) != 0);
  } else {
    total_args = n_varnames + 1;
  }
  if (total_args > n_varnames) {
    assert(false);
  }

  // printf("create code object, nfree %ld, ncell %ld\n", PyTuple_GET_SIZE(freevars), PyTuple_GET_SIZE(cellvars));

  if (n_cellvars) {
    bool used_cell2arg = false;
    cell2arg = PyMem_NEW(Py_ssize_t, n_cellvars);
    if (cell2arg == NULL) {
      assert(false);
    }
    // Find cells which are also arguments
    for (i = 0; i < n_cellvars; i++) {
      Py_ssize_t j;
      PyObject *cell = PyTuple_GET_ITEM(cellvars, i);
      cell2arg[i] = CO_CELL_NOT_AN_ARG;
      for (j = 0; j < total_args; j++) {
        PyObject *arg = PyTuple_GET_ITEM(varnames, j);
        int cmp = PyUnicode_Compare(cell, arg);
        if (cmp == -1 && PyErr_Occurred()) {
          assert(false);
        }
        if (cmp == 0) {
          cell2arg[i] = j;
          used_cell2arg = true;
          break;
        }
      }
    }
    if (!used_cell2arg) {
      PyMem_Free(cell2arg);
      cell2arg = NULL;
    }
  }

  co = PyObject_New(PyCodeObject, &PyCode_Type);
  if (co == NULL) {
    assert(false);
  }
  co->co_argcount = argcount;
  co->co_posonlyargcount = posonlyargcount;
  co->co_kwonlyargcount = kwonlyargcount;
  // printf("kwonlyargcount is %d\n", kwonlyargcount);
  co->co_nlocals = nlocals;
  co->co_stacksize = stacksize;
  co->co_flags = flags;
  Py_INCREF(code);
  co->co_code = code;
  Py_INCREF(consts);
  co->co_consts = consts;
  Py_INCREF(names);
  co->co_names = names;
  Py_INCREF(varnames);
  co->co_varnames = varnames;
  Py_INCREF(freevars);
  co->co_freevars = freevars;
  Py_INCREF(cellvars);
  co->co_cellvars = cellvars;
  co->co_cell2arg = cell2arg;
  Py_INCREF(name);
  co->co_name = name;
  co->co_firstlineno = firstlineno;
  Py_INCREF(linetable);
  co->co_linetable = linetable;
  co->co_extra = NULL;
  return co;
}

static void code_dealloc(PyCodeObject *co) {
  Py_XDECREF(co->co_code);
  Py_XDECREF(co->co_consts);
  Py_XDECREF(co->co_names);
  Py_XDECREF(co->co_varnames);
  // Py_XDECREF(co->co_filename);
  Py_XDECREF(co->co_name);
  PyObject_Free(co);
}

static Py_hash_t code_hash(PyCodeObject *co) {
  // TODO follow cpy
  Py_hash_t h, h0, h1;
  h0 = PyObject_Hash(co->co_name);
  if (h0 == -1) return -1;
  h1 = PyObject_Hash(co->co_code);
  if (h1 == -1) return -1;
  h = h0 ^ h1;
  if (h == -1) h = -2;
  return h;
}
