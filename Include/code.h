#pragma once

typedef struct PyCodeObject PyCodeObject;

#include "cpython/code.h"
#include "tupleobject.h"
#include "internal/pycore_tuple.h"
#include "cpython/pyctype.h"

#define CO_NOFREE 0x0040

typedef uint16_t _Py_CODEUNIT;

static void code_dealloc(PyCodeObject *co);

PyTypeObject PyCode_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "code",
  .tp_basicsize = sizeof(PyCodeObject),
  .tp_dealloc = (destructor) code_dealloc,
};

// defined in cpy/Objects/codeobject.c
PyObject *
_PyCode_ConstantKey(PyObject *op) {
	PyObject *key;

	if (op == Py_None || PyUnicode_CheckExact(op)) {
		Py_INCREF(op);
		key = op;
	} else if (PyBytes_CheckExact(op)) {
		key = PyTuple_Pack(2, Py_TYPE(op), op);
	} else if (PyTuple_CheckExact(op)) {
		Py_ssize_t i, len;
		PyObject *tuple;

		len = PyTuple_GET_SIZE(op);
		tuple = PyTuple_New(len);
		if (tuple == NULL)
			return NULL;

		for (i = 0; i < len; i++) {
			PyObject *item, *item_key;

			item = PyTuple_GET_ITEM(op, i);
			item_key = _PyCode_ConstantKey(item);
			if (item_key == NULL) {
				Py_DECREF(tuple);
				return NULL;
			}

			PyTuple_SET_ITEM(tuple, i, item_key);
		}

		key = PyTuple_Pack(2, tuple, op);
		Py_DECREF(tuple);
	} else {
		printf("_PyCode_ConstantKey got object of type %s\n", Py_TYPE(op)->tp_name);
		assert(false);
	}
	return key;
}

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
      assert(false);
    }
  }
  return 0;
}

PyCodeObject *
PyCode_NewWithPosOnlyArgs(int nlocals, int stacksize, int flags,
    PyObject *code, PyObject *consts,
    PyObject *names, PyObject *varnames, PyObject *name, int firstlineno,
    PyObject *linetable) {
  PyCodeObject *co;
  Py_ssize_t n_varnames;

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
  if (intern_string_constants(consts, NULL) < 0) {
    return NULL;
  }

  if (PyBytes_GET_SIZE(code) > INT_MAX) {
    assert(false);
  }

  flags |= CO_NOFREE;
  
  n_varnames = PyTuple_GET_SIZE(varnames);

  co = PyObject_New(PyCodeObject, &PyCode_Type);
  if (co == NULL) {
    assert(false);
  }
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
