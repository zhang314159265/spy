#include <objimpl.h>
#include "internal/pycore_object.h"
// #include "boolobject.h"
// #include "methodobject.h"

#define PyUnicode_GET_LENGTH(op) \
	(assert(PyUnicode_Check(op)), \
	 assert(PyUnicode_IS_READY(op)), \
	 ((PyASCIIObject *)(op))->length)

#define PyUnicode_KIND(op) \
	(assert(PyUnicode_Check(op)), \
	 assert(PyUnicode_IS_READY(op)), \
	 ((PyASCIIObject *)(op))->state.kind)

#define PyUnicode_READY(op) \
	(assert(PyUnicode_Check(op)), \
	 (PyUnicode_IS_READY(op) ? \
	  0 : \
		_PyUnicode_Ready(op)))

#define _PyUnicode_LENGTH(op) \
	(((PyASCIIObject *)(op))->length)
#define _PyUnicode_HASH(op) \
	(((PyASCIIObject *)(op))->hash)
#define _PyUnicode_STATE(op) \
	(((PyASCIIObject *)(op))->state)
#define _PyUnicode_WSTR(op) \
	(((PyASCIIObject *)(op))->wstr)

#define PyUnicode_IS_COMPACT(op) \
	(((PyASCIIObject*)(op))->state.compact)

#define PyUnicode_IS_ASCII(op) \
	(assert(PyUnicode_Check(op)), \
	 assert(PyUnicode_IS_READY(op)), \
	 ((PyASCIIObject*) op)->state.ascii)

// Fast check to determine whether an object is ready. Equivalent to
// PyUnicode_IS_COMPACT(op) || ((PyUnicodeObject*)(op))->data.any
#define PyUnicode_IS_READY(op) (((PyASCIIObject*)op)->state.ready)

#define _PyUnicode_COMPACT_DATA(op) \
	(PyUnicode_IS_ASCII(op) ? \
		((void*)((PyASCIIObject*)(op) + 1)) : \
		((void*)((PyCompactUnicodeObject*)(op) + 1)))

#define _PyUnicode_NONCOMPACT_DATA(op) \
	(assert(false), NULL)

#define PyUnicode_DATA(op) \
	(assert(PyUnicode_Check(op)), \
	 PyUnicode_IS_COMPACT(op) ? _PyUnicode_COMPACT_DATA(op) : \
	 _PyUnicode_NONCOMPACT_DATA(op))

#define PyUnicode_1BYTE_DATA(op) ((Py_UCS1*) PyUnicode_DATA(op))
#define PyUnicode_2BYTE_DATA(op) ((Py_UCS2*) PyUnicode_DATA(op))
#define PyUnicode_4BYTE_DATA(op) ((Py_UCS4*) PyUnicode_DATA(op))

typedef struct {
	PyObject_HEAD
	Py_ssize_t length; // number of code points in the string
	Py_hash_t hash; // hash value, -1 if not set
	struct {
		unsigned int interned: 2;
		unsigned int kind: 3;
		unsigned int compact: 1;
		unsigned int ascii: 1;
		unsigned int ready: 1;
		// padding
		unsigned int :24;
	} state;
	wchar_t *wstr;
} PyASCIIObject;

typedef struct {
	PyASCIIObject _base;
  Py_ssize_t utf8_length; // exclude '\0'
  char *utf8;
} PyCompactUnicodeObject;

typedef struct {
  PyCompactUnicodeObject _base;
} PyUnicodeObject;

static Py_hash_t unicode_hash(PyObject *self);
static void unicode_dealloc(PyObject *unicode);
PyObject *PyUnicode_RichCompare(PyObject *left, PyObject *right, int op);

enum PyUnicode_Kind {
	// string contains only wstr byte characters. This is only possible
	// when the string was created with a legacy API and _PyUnicode_Ready()
	// has not been called yet
	PyUnicode_WCHAR_KIND = 0,

	// return value of the PyUnicode_KIND() macro
	PyUnicode_1BYTE_KIND = 1,
	PyUnicode_2BYTE_KIND = 2,
	PyUnicode_4BYTE_KIND = 4,
};

extern PyTypeObject PyUnicode_Type;

PyObject *
PyUnicode_New(
	Py_ssize_t size, // number of code points in the new string
	Py_UCS4 maxchar // maximum code point value in the string
) {
	// Optimization for empty strings
	if (size == 0) {
    PyObject *unicode_new_empty(void);
    return unicode_new_empty();
	}

	PyObject *obj;
	PyCompactUnicodeObject *unicode;
	void *data;
	int is_ascii;
	enum PyUnicode_Kind kind;
	Py_ssize_t char_size;
	Py_ssize_t struct_size;

	is_ascii = 0;

	if (maxchar < 128) {
		kind = PyUnicode_1BYTE_KIND;
		char_size = 1;
		is_ascii = 1;
		struct_size = sizeof(PyASCIIObject);
	} else if (maxchar < 256) {
		assert(false);
	} else if (maxchar < 65536) {
		assert(false);
	} else {
		assert(false);
	}
	
	// Ensure we won't overflow the size
	if (size < 0) {
		assert(false);
	}
	obj = (PyObject *) PyObject_Malloc(struct_size + (size + 1) * char_size);
	if (obj == NULL) {
		assert(false);
	}
	_PyObject_Init(obj, &PyUnicode_Type);

	unicode = (PyCompactUnicodeObject *) obj;
	if (is_ascii) {
		data = ((PyASCIIObject*) obj) + 1;
	} else {
		data = unicode + 1;
	}
	_PyUnicode_LENGTH(unicode) = size;
	_PyUnicode_HASH(unicode) = -1;
	_PyUnicode_STATE(unicode).interned = 0;
	_PyUnicode_STATE(unicode).kind = kind;
	_PyUnicode_STATE(unicode).compact = 1;
	_PyUnicode_STATE(unicode).ready = 1;
	_PyUnicode_STATE(unicode).ascii = is_ascii;
	if (is_ascii) {
		((char *) data)[size] = 0;
		_PyUnicode_WSTR(unicode) = NULL;
	} else if (kind == PyUnicode_1BYTE_KIND) {
		assert(false);
	} else {
		assert(false);
	}

	return obj;
}

int
_PyUnicode_Ready(PyObject *unicode) {
	assert(false);
}

static void unicode_dealloc(PyObject *unicode) {
	// printf("WARNING: dealloc str object is not implemented yet\n");
}

static int
unicode_compare_eq(PyObject *str1, PyObject *str2)
{
  int kind;
  const void *data1, *data2;
  Py_ssize_t len;
  int cmp;

  len = PyUnicode_GET_LENGTH(str1);
  if (PyUnicode_GET_LENGTH(str2) != len)
    return 0;
  kind = PyUnicode_KIND(str1);
  if (PyUnicode_KIND(str2) != kind)
    return 0;
  data1 = PyUnicode_DATA(str1);
  data2 = PyUnicode_DATA(str2);

  cmp = memcmp(data1, data2, len * kind);
  return (cmp == 0);
}

PyObject *PyBool_FromLong(long ok);

static int
unicode_compare(PyObject *str1, PyObject *str2) {
  int kind1, kind2;
  const void *data1, *data2;
  Py_ssize_t len1, len2, len;

  kind1 = PyUnicode_KIND(str1);
  kind2 = PyUnicode_KIND(str2);
  data1 = PyUnicode_DATA(str1);
  data2 = PyUnicode_DATA(str2);
  len1 = PyUnicode_GET_LENGTH(str1);
  len2 = PyUnicode_GET_LENGTH(str2);
  len = Py_MIN(len1, len2);

  switch (kind1) {
  case PyUnicode_1BYTE_KIND: {
    switch (kind2) {
    case PyUnicode_1BYTE_KIND:
    {
      int cmp = memcmp(data1, data2, len);
      if (cmp < 0)
        return -1;
      if (cmp > 0)
        return 1;
      break;
    }
    default:
      assert(false);
    }
    break;
  }
  default:
    assert(false);
  }

  if (len1 == len2)
    return 0;
  if (len1 < len2)
    return -1;
  else
    return 1;
}


extern struct _longobject _Py_FalseStruct;
extern struct _longobject _Py_TrueStruct;
#define Py_False ((PyObject *) &_Py_FalseStruct)
#define Py_True ((PyObject *) &_Py_TrueStruct)
#define Py_RETURN_TRUE return Py_NewRef(Py_True)
#define Py_RETURN_FALSE return Py_NewRef(Py_False)

PyObject *PyUnicode_RichCompare(PyObject *left, PyObject *right, int op) {
  int result;

  if (!PyUnicode_Check(left) || !PyUnicode_Check(right))
    Py_RETURN_NOTIMPLEMENTED;

  if (PyUnicode_READY(left) == -1 ||
      PyUnicode_READY(right) == -1)
    return NULL;

  if (left == right) {
    switch (op) {
    case Py_NE:
      Py_RETURN_FALSE;
    default:
      fail("op %d\n", op);
    }
  } else if (op == Py_EQ || op == Py_NE) {
    result = unicode_compare_eq(left, right);
    result ^= (op == Py_NE);
    return PyBool_FromLong(result);
  } else {
    result = unicode_compare(left, right);
    Py_RETURN_RICHCOMPARE(result, 0, op);
  }
}
