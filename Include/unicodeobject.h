#pragma once

#include <stdint.h>

typedef uint32_t Py_UCS4;
typedef uint16_t Py_UCS2;
typedef uint8_t Py_UCS1;

#define _Py_RETURN_UNICODE_EMPTY() \
	do { \
		return unicode_new_empty(); \
	} while (0)

#include "cpython/fileutils.h"
#include "pyhash.h"

#define PyUnicode_Check(op) \
	PyType_FastSubclass(Py_TYPE(op), Py_TPFLAGS_UNICODE_SUBCLASS)

#define PyUnicode_CheckExact(op) Py_IS_TYPE(op, &PyUnicode_Type)

#include "cpython/unicodeobject.h"

#if (SIZEOF_SIZE_T == 8)
#define ASCII_CHAR_MASK 0x8080808080808080ULL
#elif (SIZEOF_SIZE_T == 4)
#error size_t == 4
#else
#error C 'size_t' size should be either 4 or 8!
#endif

Py_ssize_t
ascii_decode(const char *start, const char *end, Py_UCS1 *dest) {
	const char *p = start;

#if SIZEOF_SIZE_T <= SIZEOF_VOID_P
	assert(_Py_IS_ALIGNED(dest, ALIGNOF_SIZE_T));
	if (_Py_IS_ALIGNED(p, ALIGNOF_SIZE_T)) {
		const char *_p = p;
		Py_UCS1 *q = dest;
		while (_p + SIZEOF_SIZE_T <= end) {
			size_t value = *(const size_t *) _p;
			if (value & ASCII_CHAR_MASK) {
				break;
			}
			*((size_t *) q) = value;
			_p += SIZEOF_SIZE_T;
			q += SIZEOF_SIZE_T;
		}
		p = _p;
		while (p < end) {
			if ((unsigned char) *p & 0x80) {
				break;
			}
			*q++ = *p++;
		}
		return p - start;
	}
#endif
	while (p < end) {
		if (_Py_IS_ALIGNED(p, ALIGNOF_SIZE_T)) {
			const char *_p = p;
			while (_p + SIZEOF_SIZE_T <= end) {
				size_t value = *(const size_t *) _p;
				if (value & ASCII_CHAR_MASK) {
					break;
				}
				_p += SIZEOF_SIZE_T;
			}
			p = _p;
			if (_p == end) {
				break;
			}
		}
		if ((unsigned char) *p & 0x80) {
			break;
		}
		++p;
	}
	memcpy(dest, start, p - start);
	return p - start;
}

PyObject *unicode_get_empty() {
	// TODO cache it
	PyObject *empty = PyUnicode_New(1, 0);
	if (empty == NULL) {
		assert(false);
	}
	PyUnicode_1BYTE_DATA(empty)[0] = 0;
	_PyUnicode_LENGTH(empty) = 0;
	return empty;
}

// Return a strong reference to the empty string singleton
PyObject *unicode_new_empty(void) {
	PyObject *empty = unicode_get_empty();
	Py_INCREF(empty);
	return empty;
}

PyObject *
unicode_decode_utf8(const char *s, Py_ssize_t size,
		_Py_error_handler error_handler, const char *errors,
		Py_ssize_t *consumed) {
	if (size == 0) {
		if (consumed) {
			*consumed = 0;
		}
		_Py_RETURN_UNICODE_EMPTY();
	}
  #if 0
	// ASCII is equivalent to the first 128 ordinals in Unicode
	if (size == 1 && (unsigned char) s[0] < 128) {
		assert(false);
	}
  #endif

	const char *starts = s;
	const char *end = s + size;

	// fast path: try ASCII string.
	PyObject *u = PyUnicode_New(size, 127);
	if (u == NULL) {
		return NULL;
	}
	s += ascii_decode(s, end, PyUnicode_1BYTE_DATA(u));
	if (s == end) {
		return u;
	}
	assert(false);
}

PyObject *
PyUnicode_DecodeUTF8Stateful(const char *s,
		Py_ssize_t size,
		const char *errors,
		Py_ssize_t *consumed) {
  return unicode_decode_utf8(s, size, _Py_ERROR_UNKNOWN, errors, consumed);
}

PyObject *
PyUnicode_DecodeUTF8(
	const char *s, /* UTF-8 encoded string */
	Py_ssize_t size, /* size of string */
	const char *errors /* error handling */
) {
	return PyUnicode_DecodeUTF8Stateful(s, size, errors, NULL);
}

Py_hash_t
unicode_hash(PyObject *self) {
	Py_uhash_t x; /* Unsigned for defined overflow behavior. */
	if (_PyUnicode_HASH(self) != -1) {
		return _PyUnicode_HASH(self);
	}
	if (PyUnicode_READY(self) == -1) {
		return -1;
	}
	x = _Py_HashBytes(PyUnicode_DATA(self),
			PyUnicode_GET_LENGTH(self) * PyUnicode_KIND(self));
	_PyUnicode_HASH(self) = x;
	return x;
}

void
PyUnicode_InternInPlace(PyObject **p) {
	PyObject *s = *p;
	(void) unicode_hash(s);
}

PyObject *
PyUnicode_FromString(const char *u) {
	size_t size = strlen(u);
	return PyUnicode_DecodeUTF8Stateful(u, (Py_ssize_t) size, NULL, NULL);
}

PyObject *PyUnicode_InternFromString(const char *cp) {
	PyObject *s = PyUnicode_FromString(cp);
	if (s == NULL)
		return NULL;
	PyUnicode_InternInPlace(&s);
	return s;
}

PyObject * _PyUnicode_FromId(_Py_Identifier *id);

const char *
PyUnicode_AsUTF8AndSize(PyObject *unicode, Py_ssize_t *psize) {
  assert(false);
}

const char *
PyUnicode_AsUTF8(PyObject *unicode)
{
  return PyUnicode_AsUTF8AndSize(unicode, NULL);
}

int
_PyUnicode_EqualToASCIIString(PyObject *unicode, const char *str) {
  size_t len;
  // assert(_PyUnicode_CHECK(unicode));

  if (PyUnicode_READY(unicode) == -1) {
    assert(false);
  }
  if (!PyUnicode_IS_ASCII(unicode))
    return 0;
  len = (size_t) PyUnicode_GET_LENGTH(unicode);
  return strlen(str) == len &&
      memcmp(PyUnicode_1BYTE_DATA(unicode), str, len) == 0;
}
