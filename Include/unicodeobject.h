#pragma once

#include <stdint.h>
#include <stdarg.h>

typedef uint32_t Py_UCS4;
typedef uint16_t Py_UCS2;
typedef uint8_t Py_UCS1;

#define PyUnicode_CHECK_INTERNED(op) \
    (((PyASCIIObject *)(op))->state.interned)

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

#if 1
#define PyUnicode_WRITE(kind, data, index, value) \
  do { \
    switch ((kind)) { \
    case PyUnicode_1BYTE_KIND: { \
      ((Py_UCS1 *)(data))[(index)] = (Py_UCS1)(value); \
      break; \
    } \
    default: \
      assert(false); \
    } \
  } while (0)
#endif

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

// defined in cpy/Include/cpython/unicodeobject.h
typedef struct {
  unsigned char overallocate;
  Py_ssize_t min_length;

  Py_UCS4 min_char;
  Py_UCS4 maxchar;
  enum PyUnicode_Kind kind;

  Py_ssize_t size;
  Py_ssize_t pos;

  PyObject *buffer;

  // If readonly is 1, buffer is a shared string (cannot be modified)
  // and size is set to 0.
  unsigned char readonly;

  void *data;
} _PyUnicodeWriter;

// defined in cpy/Objects/unicodeobject.c
void
_PyUnicodeWriter_Init(_PyUnicodeWriter *writer) {
  memset(writer, 0, sizeof(*writer));

  writer->min_char = 127;
  writer->kind = PyUnicode_WCHAR_KIND;
  assert(writer->kind <= PyUnicode_1BYTE_KIND);
}

#define _PyUnicode_CHECK(op) PyUnicode_Check(op)

#define _PyUnicode_SHARE_WSTR(op) \
  (assert(_PyUnicode_CHECK(op)), \
   (_PyUnicode_WSTR(unicode) == PyUnicode_DATA(op)))

#define _PyUnicode_UTF8(op) \
  (((PyCompactUnicodeObject*)(op))->utf8)

#define PyUnicode_UTF8(op) \
  (assert(_PyUnicode_CHECK(op)), \
   assert(PyUnicode_IS_READY(op)), \
   PyUnicode_IS_COMPACT_ASCII(op) ? \
     ((char *) ((PyASCIIObject*) (op) + 1)) : \
     _PyUnicode_UTF8(op))

#define _PyUnicode_HAS_UTF8_MEMORY(op) \
  ((!PyUnicode_IS_COMPACT_ASCII(op) \
    && _PyUnicode_UTF8(op) \
    && _PyUnicode_UTF8(op) != PyUnicode_DATA(op)))

#define PyUnicode_IS_COMPACT_ASCII(op) \
  (((PyASCIIObject *) op)->state.ascii && PyUnicode_IS_COMPACT(op))

#define _PyUnicode_HAS_WSTR_MEMORY(op) \
  ((_PyUnicode_WSTR(op) && \
   (!PyUnicode_IS_READY(op) || \
    _PyUnicode_WSTR(op) != PyUnicode_DATA(op))))

#define _PyUnicode_UTF8_LENGTH(op) \
  (((PyCompactUnicodeObject *)(op))->utf8_length)

#define PyUnicode_UTF8_LENGTH(op) \
  (assert(_PyUnicode_CHECK(op)), \
   assert(PyUnicode_IS_READY(op)), \
   PyUnicode_IS_COMPACT_ASCII(op) ? \
     ((PyASCIIObject *)(op))->length : \
     _PyUnicode_UTF8_LENGTH(op))

const char *
PyUnicode_AsUTF8AndSize(PyObject *unicode, Py_ssize_t *psize) {
  if (!PyUnicode_Check(unicode)) {
    assert(false);
  }
  if (PyUnicode_READY(unicode) == -1)
    return NULL;

  if (PyUnicode_UTF8(unicode) == NULL) {
    assert(false);
  }

  if (psize)
    *psize = PyUnicode_UTF8_LENGTH(unicode);
  return PyUnicode_UTF8(unicode);
}

const char *
PyUnicode_AsUTF8(PyObject *unicode)
{
  return PyUnicode_AsUTF8AndSize(unicode, NULL);
}

static PyObject *
resize_compact(PyObject *unicode, Py_ssize_t length) {
  Py_ssize_t char_size;
  Py_ssize_t struct_size;
  Py_ssize_t new_size;
  int share_wstr;
  PyObject *new_unicode;

  char_size = PyUnicode_KIND(unicode);
  if (PyUnicode_IS_ASCII(unicode)) {
    struct_size = sizeof(PyASCIIObject);
  } else {
    assert(false);
  }
  share_wstr = _PyUnicode_SHARE_WSTR(unicode);

  new_size = (struct_size + (length + 1) * char_size);
  if (_PyUnicode_HAS_UTF8_MEMORY(unicode)) {
    assert(false);
  }
  new_unicode = (PyObject *) PyObject_Realloc(unicode, new_size);
  if (new_unicode == NULL) {
    assert(false);
  }
  unicode = new_unicode;
  _Py_NewReference(unicode);

  _PyUnicode_LENGTH(unicode) = length;
  if (share_wstr) {
    assert(false);
  } else if (_PyUnicode_HAS_WSTR_MEMORY(unicode)) {
    assert(false);
  }
  PyUnicode_WRITE(PyUnicode_KIND(unicode), PyUnicode_DATA(unicode),
    length, 0);
  return unicode;
}

static PyObject *
unicode_result_ready(PyObject *unicode) {
  // TODO follow cpy
  return unicode;
}

PyObject *
_PyUnicodeWriter_Finish(_PyUnicodeWriter *writer) {
  PyObject *str;

  if (writer->pos == 0) {
    assert(false);
  }

  str = writer->buffer;
  writer->buffer = NULL;

  if (writer->readonly) {
    assert(false);
  }

  if (PyUnicode_GET_LENGTH(str) != writer->pos) {
    PyObject *str2;
    str2 = resize_compact(str, writer->pos);
    if (str2 == NULL) {
      Py_DECREF(str);
      return NULL;
    }
    str = str2;
  }

  return unicode_result_ready(str);
}

#define PyUnicode_READ_CHAR(unicode, index) \
  (assert(PyUnicode_Check(unicode)), \
   assert(PyUnicode_IS_READY(unicode)), \
   (Py_UCS4) \
    (PyUnicode_KIND((unicode)) == PyUnicode_1BYTE_KIND ? \
     ((const Py_UCS1 *) (PyUnicode_DATA((unicode))))[(index)] : \
     (PyUnicode_KIND((unicode)) == PyUnicode_2BYTE_KIND ? \
      ((const Py_UCS2 *) (PyUnicode_DATA((unicode))))[(index)] : \
      ((const Py_UCS4 *) (PyUnicode_DATA((unicode))))[(index)] \
     ) \
   ))

int _PyUnicodeWriter_WriteSubstring(_PyUnicodeWriter *writer, PyObject *str, Py_ssize_t start, Py_ssize_t end);

// an efficient approximation
#define PyUnicode_MAX_CHAR_VALUE(op) \
  (assert(PyUnicode_IS_READY(op)), \
   (PyUnicode_IS_ASCII(op) ? \
    (0x7f) : \
    (PyUnicode_KIND(op) == PyUnicode_1BYTE_KIND ? \
     (0xffU) : \
     (PyUnicode_KIND(op) == PyUnicode_2BYTE_KIND ? \
      (0xffffU) : \
      (0x10ffffU)))))

// Prepare the buffer to writer 'length' characters
// with the specified maximum character.
#define _PyUnicodeWriter_Prepare(WRITER, LENGTH, MAXCHAR) \
    (((MAXCHAR) <= (WRITER)->maxchar \
      && (LENGTH) <= (WRITER)->size - (WRITER)->pos) \
      ? 0 \
      : (((LENGTH) == 0) \
        ? 0 \
        : _PyUnicodeWriter_PrepareInternal((WRITER), (LENGTH), (MAXCHAR))))

static inline void
_PyUnicodeWriter_Update(_PyUnicodeWriter *writer) {
  writer->maxchar = PyUnicode_MAX_CHAR_VALUE(writer->buffer);
  writer->data = PyUnicode_DATA(writer->buffer);

  if (!writer->readonly) {
    writer->kind = PyUnicode_KIND(writer->buffer);
    writer->size = PyUnicode_GET_LENGTH(writer->buffer);
  } else {
    assert(false);
  }
}

#define OVERALLOCATE_FACTOR 4

// defined in cpy/Objects/unicodeobject.c
int
_PyUnicodeWriter_PrepareInternal(_PyUnicodeWriter *writer,
    Py_ssize_t length, Py_UCS4 maxchar) {
  Py_ssize_t newlen;
  PyObject *newbuffer;

  assert((maxchar > writer->maxchar && length >= 0) || length > 0);

  newlen = writer->pos + length;
  maxchar = Py_MAX(maxchar, writer->min_char);

  if (writer->buffer == NULL) {
    assert(!writer->readonly);
    if (writer->overallocate) {
      newlen += newlen / OVERALLOCATE_FACTOR;
    }
    if (newlen < writer->min_length)
      newlen = writer->min_length;

    writer->buffer = PyUnicode_New(newlen, maxchar);
    if (writer->buffer == NULL)
      return -1;
  }
  else if (newlen > writer->size) {
    if (writer->overallocate) {
      newlen += newlen / OVERALLOCATE_FACTOR;
    }
    if (newlen < writer->min_length)
      newlen = writer->min_length;

    if (maxchar > writer->maxchar || writer->readonly) {
      assert(false);
    } else {
      newbuffer = resize_compact(writer->buffer, newlen);
      if (newbuffer == NULL)
        return -1;
    }
    writer->buffer = newbuffer;
  }
  else if (maxchar > writer->maxchar) {
    assert(false);
  }
  _PyUnicodeWriter_Update(writer);
  return 0;
}

PyObject *_PyUnicode_FromASCII(const char *buffer, Py_ssize_t size) {
  const unsigned char *s = (const unsigned char *) buffer;
  PyObject *unicode;
  #if 0
  if (size == 1) {
    assert(false);
  }
  #endif
  unicode = PyUnicode_New(size, 127);
  if (!unicode)
    return NULL;
  memcpy(PyUnicode_1BYTE_DATA(unicode), s, size);
  return unicode;
}

void _PyUnicode_FastCopyCharacters(
		PyObject *to, Py_ssize_t to_start,
		PyObject *from, Py_ssize_t from_start, Py_ssize_t how_many);

int _PyUnicodeWriter_WriteStr(_PyUnicodeWriter *writer, PyObject *str) {
  Py_UCS4 maxchar;
  Py_ssize_t len;

  if (PyUnicode_READY(str) == -1)
    return -1;
  len = PyUnicode_GET_LENGTH(str);
  if (len == 0)
    return 0;

  maxchar = PyUnicode_MAX_CHAR_VALUE(str);
  if (maxchar > writer->maxchar || len > writer->size - writer->pos) {
    if (writer->buffer == NULL && !writer->overallocate) {
      assert(false);
    }
    if (_PyUnicodeWriter_PrepareInternal(writer, len, maxchar) == -1)
      return -1;
  }
  _PyUnicode_FastCopyCharacters(writer->buffer, writer->pos,
      str, 0, len);
  writer->pos += len;
  return 0;
}

static inline int
_PyUnicodeWriter_WriteCharInline(_PyUnicodeWriter *writer, Py_UCS4 ch) {
  if (_PyUnicodeWriter_Prepare(writer, 1, ch) < 0) {
    return -1;
  }
  PyUnicode_WRITE(writer->kind, writer->data, writer->pos, ch);
  writer->pos++;
  return 0;
}

int _PyUnicodeWriter_WriteChar(_PyUnicodeWriter *writer, Py_UCS4 ch) {
  return _PyUnicodeWriter_WriteCharInline(writer, ch);
}

int
_PyUnicodeWriter_WriteASCIIString(_PyUnicodeWriter *writer,
    const char *ascii, Py_ssize_t len) {
  if (len == -1)
    len = strlen(ascii);

  if (writer->buffer == NULL && !writer->overallocate) {
    assert(false);
  }

  if (_PyUnicodeWriter_Prepare(writer, len, 127) == -1)
    return -1;

  switch (writer->kind) {
  case PyUnicode_1BYTE_KIND:
  {
    const Py_UCS1 *str = (const Py_UCS1 *) ascii;
    Py_UCS1 *data = writer->data;
    memcpy(data + writer->pos, str, len);
    break;
  }
  default:
    assert(false);
  }

  writer->pos += len;
  return 0;
}

PyObject *
PyUnicode_Substring(PyObject *self, Py_ssize_t start, Py_ssize_t end) {
  const unsigned char *data;
  Py_ssize_t length;

  if (PyUnicode_READY(self) == -1)
    return NULL;

  length = PyUnicode_GET_LENGTH(self);
  end = Py_MIN(end, length);

  if (start == 0 && end == length) {
    assert(false);
  }

  if (start < 0 || end < 0) {
    assert(false);
  }

  if (start >= length || end < start) {
    assert(false);
  }

  length = end - start;
  if (PyUnicode_IS_ASCII(self)) {
    data = PyUnicode_1BYTE_DATA(self);
    return _PyUnicode_FromASCII((const char *) (data + start), length);
  } else {
    assert(false);
  }
}

static int
unicode_fromformat_write_str(_PyUnicodeWriter *writer, PyObject *str,
    Py_ssize_t width, Py_ssize_t precision) {
  Py_ssize_t length;

  if (PyUnicode_READY(str) == -1)
    return -1;

  length = PyUnicode_GET_LENGTH(str);
  if ((precision == -1 || precision >= length) && width <= length) {
    return _PyUnicodeWriter_WriteStr(writer, str);
  }
  assert(false);
}

static int
unicode_fromformat_write_cstr(_PyUnicodeWriter *writer, const char *str,
    Py_ssize_t width, Py_ssize_t precision) {
  Py_ssize_t length;
  PyObject *unicode;
  int res;

  if (precision == -1) {
    length = strlen(str);
  } else {
    assert(false);
  }
  unicode = PyUnicode_DecodeUTF8Stateful(str, length, "replace", NULL);
  if (unicode == NULL)
    return -1;

  res = unicode_fromformat_write_str(writer, unicode, width, -1);
  Py_DECREF(unicode);
  return res;
}

static const char*
unicode_fromformat_arg(_PyUnicodeWriter *writer,
    const char *f, va_list *vargs)
{
  const char *p;
  int zeropad;
  Py_ssize_t width;
  Py_ssize_t precision;
  int longflag;
  int longlongflag;
  int size_tflag;

  p = f;
  f++;
  zeropad = 0;
  if (*p == '0') {
    zeropad = 1;
    f++;
  }

  width = -1;
  if (isdigit((unsigned) *f)) {
    assert(false);
  }
  precision = -1;
  if (*f == '.') {
    assert(false);
  }
  if (*f == '\0') {
    assert(false);
  }
  longflag = 0;
  longlongflag = 0;
  size_tflag = 0;
  if (*f == 'l') {
    assert(false);
  }
  else if (*f == 'z' && (f[1] == 'd' || f[1] == 'u' || f[1] == 'i')) {
    size_tflag = 1;
    ++f;
  }

  if (f[1] == '\0')
    writer->overallocate = 0;

  switch (*f) {
  case 'U': {
    PyObject *obj = va_arg(*vargs, PyObject *);
    assert(obj && _PyUnicode_CHECK(obj));

    if (unicode_fromformat_write_str(writer, obj, width, precision) == -1)
      return NULL;
    break;
  }
  case 's':
  {
    const char *s = va_arg(*vargs, const char *);
    if (unicode_fromformat_write_cstr(writer, s, width, precision) < 0)
      return NULL;
    break;
  }
  default:
    assert(false);
  }
  f++;
  return f;
}

PyObject *
PyUnicode_FromFormatV(const char *format, va_list vargs) {
  va_list vargs2;
  const char *f;
  _PyUnicodeWriter writer;

  _PyUnicodeWriter_Init(&writer);
  writer.min_length = strlen(format) + 100;
  writer.overallocate = 1;

  va_copy(vargs2, vargs);

  for (f = format; *f; ) {
    if (*f == '%') {
      f = unicode_fromformat_arg(&writer, f, &vargs2);
      if (f == NULL)
        assert(false);
    } else {
      const char *p;
      Py_ssize_t len;

      p = f;
      do {
        if ((unsigned char) *p > 127) {
          assert(false);
        }
        p++;
      } while (*p != '\0' && *p != '%');
      len = p - f;

      if (*p == '\0')
        writer.overallocate = 0;

      if (_PyUnicodeWriter_WriteASCIIString(&writer, f, len) < 0)
        assert(false);

      f = p;
    }
  }
  va_end(vargs2);
  return _PyUnicodeWriter_Finish(&writer);
}

PyObject *
PyUnicode_FromFormat(const char *format, ...) {
  PyObject *ret;
  va_list vargs;

  va_start(vargs, format);

  ret = PyUnicode_FromFormatV(format, vargs);
  va_end(vargs);
  return ret;
}

int unicode_eq(PyObject *aa, PyObject *bb);

int
_PyUnicode_EQ(PyObject *aa, PyObject *bb) {
  return unicode_eq(aa, bb);
}

int PyUnicode_Compare(PyObject *left, PyObject *right) {
  if (PyUnicode_Check(left) && PyUnicode_Check(right)) {
    if (PyUnicode_READY(left) == -1 ||
        PyUnicode_READY(right) == -1)
      return -1;

    if (left == right)
      return 0;

    return unicode_compare(left, right);
  }
  assert(false);
}
