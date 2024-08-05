#pragma once

#if PYLONG_BITS_IN_DIGIT == 30
typedef uint32_t digit;
typedef int32_t sdigit; // signed variant of digit
#define PyLong_SHIFT 30
#elif PYLONG_BITS_IN_DIGIT == 15
typedef unsigned short digit;
typedef short sdigit;
#define PyLong_SHIFT 15
#else
#error "PYLONG_BITS_IN_DIGIT should be 15 or 30"
#endif

#define PyLong_BASE ((digit) 1 << PyLong_SHIFT)
#define PyLong_MASK ((digit) (PyLong_BASE - 1))

struct _longobject {
  PyObject_VAR_HEAD
  digit ob_digit[1];
};

static Py_hash_t long_hash(PyLongObject *v);
static PyObject *long_richcompare(PyObject *self, PyObject *other, int op);

// defined in cpy/Objects/longobject.c
PyTypeObject PyLong_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "int",
  .tp_basicsize = offsetof(PyLongObject, ob_digit),
  .tp_flags = Py_TPFLAGS_LONG_SUBCLASS,
  .tp_free = PyObject_Del,
  .tp_hash = (hashfunc) long_hash,
  .tp_richcompare = long_richcompare,
};

// defined in cpy/Objects/longobject.c
PyLongObject * _PyLong_New(Py_ssize_t size) {
  PyLongObject * result;
  result = PyObject_Malloc(offsetof(PyLongObject, ob_digit) +
    size * sizeof(digit));
  if (!result) {
    assert(false);
  }
  _PyObject_InitVar((PyVarObject*) result, &PyLong_Type, size);
  return result;
}

static Py_hash_t
long_hash(PyLongObject *v) {
  Py_uhash_t x;
  Py_ssize_t i;
  int sign;

  i = Py_SIZE(v);
  switch (i) {
  case -1: return v->ob_digit[0] == 1 ? -2 : -(sdigit)v->ob_digit[0];
  case 0: return 0;
  case 1: return v->ob_digit[0];
  }
  sign = 1;
  x = 0;
  if (i < 0) {
    sign = -1;
    i = -(i);
  }
  while (--i >= 0) {
    x = ((x << PyLong_SHIFT) & _PyHASH_MODULUS) |
      (x >> (_PyHASH_BITS - PyLong_SHIFT));
    x += v->ob_digit[i];
    if (x >= _PyHASH_MODULUS)
      x -= _PyHASH_MODULUS;
  }
  x = x * sign;
  if (x == (Py_uhash_t) -1)
    x = (Py_uhash_t) -2;
  return (Py_hash_t) x;
}

#define CHECK_BINOP(v, w) \
  do { \
    if (!PyLong_Check(v) || !PyLong_Check(w)) \
      Py_RETURN_NOTIMPLEMENTED; \
  } while (0)

#include "boolobject.h"

static Py_ssize_t
long_compare(PyLongObject *a, PyLongObject *b)
{
  Py_ssize_t sign = Py_SIZE(a) - Py_SIZE(b);
  if (sign == 0) {
    Py_ssize_t i = Py_ABS(Py_SIZE(a));
    sdigit diff = 0;
    while (--i >= 0) {
      diff = (sdigit) a->ob_digit[i] - (sdigit) b->ob_digit[i];
      if (diff) {
        break;
      }
    }
    sign = Py_SIZE(a) < 0 ? -diff : diff;
  }
  return sign;
}

static PyObject *long_richcompare(PyObject *self, PyObject *other, int op) {
  Py_ssize_t result;
  CHECK_BINOP(self, other);
  if (self == other)
    result = 0;
  else
    result = long_compare((PyLongObject *) self, (PyLongObject *) other);
  Py_RETURN_RICHCOMPARE(result, 0, op);
}
