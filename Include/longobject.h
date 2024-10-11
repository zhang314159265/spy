#pragma once

typedef struct _longobject PyLongObject;

#include "internal/pycore_interp.h"
#include "internal/pycore_long.h"

#define NSMALLNEGINTS _PY_NSMALLNEGINTS
#define NSMALLPOSINTS _PY_NSMALLPOSINTS

#define IS_SMALL_INT(ival) (-NSMALLNEGINTS <= (ival) && (ival) < NSMALLPOSINTS)
#define IS_SMALL_UINT(ival) ((ival) < NSMALLPOSINTS)

#define PyLong_AS_LONG(op) PyLong_AsLong(op);

#define PyLong_Check(op) \
  PyType_FastSubclass(Py_TYPE(op), Py_TPFLAGS_LONG_SUBCLASS)

#define PyLong_CheckExact(op) Py_IS_TYPE(op, &PyLong_Type)

#include "longintrepr.h"

long
PyLong_AsLongAndOverflow(PyObject *vv, int *overflow) {
  PyLongObject *v;
  int do_decref = 0;
  long res;
  Py_ssize_t i;

  *overflow = 0;
  if (vv == NULL) {
    assert(false);
  }

  if (PyLong_Check(vv)) {
    v = (PyLongObject *) vv;
  } else {
    assert(false);
  }

  res = -1;
  i = Py_SIZE(v);
  switch (i) {
  case -1:
    res = -(sdigit) v->ob_digit[0];
    break;
  case 0:
    res = 0;
    break;
  case 1:
    res = v->ob_digit[0];
    break;
  default:
    assert(false);
  }

  if (do_decref) {
    Py_DECREF(v);
  }
  return res;
}

long PyLong_AsLong(PyObject *obj) {
  int overflow;
  long result = PyLong_AsLongAndOverflow(obj, &overflow);
  if (overflow) {
    assert(false);
  }
  return result;
}

static PyObject *
get_small_int(sdigit ival) {
  assert(IS_SMALL_INT(ival));
  PyObject *v = __PyLong_GetSmallInt_internal(ival);
  Py_INCREF(v);
  return v;
}

// defined in cpy/Objects/longobject.c
PyObject *PyLong_FromLong(long ival) {
  PyLongObject *v;
  unsigned long abs_ival;
  int sign;

  if (IS_SMALL_INT(ival)) {
    return get_small_int((sdigit) ival);
  }

  if (ival < 0) {
    abs_ival = 0U - (unsigned long) ival;
    sign = -1;
  } else {
    abs_ival = (unsigned long) ival;
    sign = ival == 0 ? 0 : 1;
  }

  // Fast path for single-digit ints
  if (!(abs_ival >> PyLong_SHIFT)) {
    v = _PyLong_New(1);
    if (v) {
      Py_SET_SIZE(v, sign);
      v->ob_digit[0] = Py_SAFE_DOWNCAST(
        abs_ival, unsigned long, digit);
    }
    return (PyObject *) v;
  }
  assert(false);
}

#define PYLONG_FROM_UINT(INT_TYPE, ival) \
  do { \
    if (IS_SMALL_UINT(ival)) { \
      return get_small_int((sdigit) (ival)); \
    } \
    /* Count the number of Python digits */ \
    Py_ssize_t ndigits = 0; \
    INT_TYPE t = (ival); \
    while (t) { \
      ++ndigits; \
      t >>= PyLong_SHIFT; \
    } \
    PyLongObject *v = _PyLong_New(ndigits); \
    if (v == NULL) { \
      return NULL; \
    } \
    digit *p = v->ob_digit; \
    while ((ival)) { \
      *p++ = (digit)((ival) & PyLong_MASK); \
      (ival) >>= PyLong_SHIFT; \
    } \
    return (PyObject *) v; \
  } while (0)

PyObject *
PyLong_FromUnsignedLong(unsigned long ival)
{
  PYLONG_FROM_UINT(unsigned long, ival);
}

PyObject *PyLong_FromVoidPtr(void *p) {
  return PyLong_FromUnsignedLong((unsigned long)(uintptr_t)p);
}

PyObject *
PyLong_FromSsize_t(Py_ssize_t ival) {
  if (IS_SMALL_INT(ival)) {
    return get_small_int((sdigit) ival);
  }
  assert(false);
}

long PyOS_strtol(const char *str, char **ptr, int base) {
	return strtol(str, ptr, base);
}

static inline PyObject *_PyLong_GetZero(void) {
	return __PyLong_GetSmallInt_internal(0);
}

static inline PyObject *_PyLong_GetOne(void) {
	return __PyLong_GetSmallInt_internal(1);
}
