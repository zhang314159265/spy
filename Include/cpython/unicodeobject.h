#include "objimpl.h"
#include "internal/pycore_object.h"

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

PyTypeObject PyUnicode_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "str",
	.tp_flags = Py_TPFLAGS_UNICODE_SUBCLASS,
};

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
} PyCompactUnicodeObject;

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

PyObject *
PyUnicode_New(
	Py_ssize_t size, // number of code points in the new string
	Py_UCS4 maxchar // maximum code point value in the string
) {
	// Optimization for empty strings
	if (size == 0) {
		assert(false);
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
