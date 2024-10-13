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

int _PyLong_FormatAdvancedWriter(_PyUnicodeWriter *writer,
		PyObject *obj,
		PyObject *format_spec,
		Py_ssize_t start, Py_ssize_t end);

static int
long_to_decimal_string_internal(PyObject *aa,
		PyObject **p_output,
		_PyUnicodeWriter *writer,
		_PyBytesWriter *bytes_writer,
		char **bytes_str) {
	PyLongObject *scratch, *a;
	PyObject *str = NULL;
	Py_ssize_t size, strlen, size_a, i, j;
	digit *pout, *pin, rem, tenpow;
	int negative;
	int d;
	enum PyUnicode_Kind kind;

	a = (PyLongObject *) aa;
	if (a == NULL || !PyLong_Check(a)) {
		assert(false);
	}
	size_a = Py_ABS(Py_SIZE(a));
	negative = Py_SIZE(a) < 0;

	// quick and dirty upper bound for the number of digits
	// required to express a in base _PyLong_DECIMAL_BASE
	d = (33 * _PyLong_DECIMAL_SHIFT) /
		(10 * PyLong_SHIFT - 33 * _PyLong_DECIMAL_SHIFT);
	size = 1 + size_a + size_a / d;
	scratch = _PyLong_New(size);
	if (scratch == NULL)
		return -1;

	// convert array of base _PyLong_BASE digits in pin to an array
	// of base _PyLong_DECIMAL_BASE digits in pout.
	pin = a->ob_digit;
	pout = scratch->ob_digit;
	size = 0;

	for (i = size_a; --i >= 0; ) {
		digit hi = pin[i];
		for (j = 0; j < size; j++) {
			assert(false);
		}
		while (hi) {
			pout[size++] = hi % _PyLong_DECIMAL_BASE;
			hi /= _PyLong_DECIMAL_BASE;
		}
	}

	if (size == 0)
		pout[size++] = 0;
	
	// calculate exact length of output string, and allocate
	strlen = negative + 1 + (size - 1) * _PyLong_DECIMAL_SHIFT;
	tenpow = 10;
	rem = pout[size - 1];
	while (rem >= tenpow) {
		tenpow *= 10;
		strlen++;
	}

	if (writer) {
		if (_PyUnicodeWriter_Prepare(writer, strlen, '9') == -1) {
			assert(false);
		}
		kind = writer->kind;
	} else if (bytes_writer) {
		assert(false);
	} else {
		str = PyUnicode_New(strlen, '9');
		if (str == NULL) {
			Py_DECREF(scratch);
			return -1;
		}
		kind = PyUnicode_KIND(str);
	}

#define WRITE_DIGITS(p) \
	do { \
		for (i = 0; i < size - 1; i++) { \
			rem = pout[i]; \
			for (j = 0; j < _PyLong_DECIMAL_SHIFT; j++) { \
				*--p = '0' + rem % 10; \
				rem /= 10; \
			} \
		} \
		rem = pout[i]; \
		do { \
			*--p = '0' + rem % 10; \
			rem /= 10; \
		} while (rem != 0); \
		\
		if (negative) \
			*--p = '-'; \
	} while (0)

#define WRITE_UNICODE_DIGITS(TYPE) \
	do { \
		if (writer) \
			p = (TYPE*) PyUnicode_DATA(writer->buffer) + writer->pos + strlen; \
		else \
			p = (TYPE*) PyUnicode_DATA(str) + strlen; \
		\
		WRITE_DIGITS(p); \
		\
		if (writer) \
			assert(p == ((TYPE *) PyUnicode_DATA(writer->buffer) + writer->pos)); 	else \
				assert(p == (TYPE *) PyUnicode_DATA(str)); \
	} while (0)

	// fill the string right-to-left
	if (bytes_writer) {
		assert(false);
	} else if (kind == PyUnicode_1BYTE_KIND) {
		Py_UCS1 *p;
		WRITE_UNICODE_DIGITS(Py_UCS1);
	} else if (kind == PyUnicode_2BYTE_KIND) {
		assert(false);
	} else {
		assert(false);
	}

#undef WRITE_DIGITS
#undef WRITE_UNICODE_DIGITS

	Py_DECREF(scratch);
	if (writer) {
		writer->pos += strlen;
	} else if (bytes_writer) {
		(*bytes_str) += strlen;
	} else {
		*p_output = (PyObject *) str;
	}
	return 0;
}

int
_PyLong_FormatWriter(_PyUnicodeWriter *writer,
		PyObject *obj,
		int base, int alternate) {
	if (base == 10)
		return long_to_decimal_string_internal(obj, NULL, writer, NULL, NULL);
	else
		assert(false);
}

double PyLong_AsDouble(PyObject *v) {
	if (v == NULL) {
		assert(false);
	}

	if (!PyLong_Check(v)) {
		assert(false);
	}
	if (Py_ABS(Py_SIZE(v)) <= 1) {
		return (double) MEDIUM_VALUE((PyLongObject *) v);
	}
	assert(false);
}

PyObject *
PyLong_FromLongLong(long long ival) {
	PyLongObject *v;
	unsigned long long abs_ival;
	unsigned long long t;
	int ndigits = 0;
	int negative = 0;

	if (ival < 0) {
		assert(false);
	} else {
		abs_ival = (unsigned long long) ival;
	}

	t = abs_ival;
	while (t) {
		++ndigits;
		t >>= PyLong_SHIFT;
	}
	v = _PyLong_New(ndigits);
	if (v != NULL) {
		digit *p = v->ob_digit;
		Py_SET_SIZE(v, negative ? -ndigits : ndigits);
		t = abs_ival;
		while (t) {
			*p++ = (digit)(t & PyLong_MASK);
			t >>= PyLong_SHIFT;
		}
	}
	return (PyObject *) v;
}
