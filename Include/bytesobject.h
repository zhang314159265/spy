#ifndef Py_BYTESOBJECT_H
#define Py_BYTESOBJECT_H

#include <stddef.h> // for offsetof
#include "cpython/bytesobject.h"
#include "objimpl.h"
#include "internal/pycore_object.h"

#define PyBytesObject_SIZE (offsetof(PyBytesObject, ob_sval) + 1)

#define PyBytes_Check(op) \
	PyType_FastSubclass(Py_TYPE(op), Py_TPFLAGS_BYTES_SUBCLASS)

#define PyBytes_CheckExact(op) Py_IS_TYPE(op, &PyBytes_Type)

#define PyBytes_GET_SIZE(op) (assert(PyBytes_Check(op)), Py_SIZE(op))

#define PyBytes_AS_STRING(op) (assert(PyBytes_Check(op)), \
		(((PyBytesObject *)(op))->ob_sval))

static Py_hash_t bytes_hash(PyBytesObject *a);

// defined in cpy/Objects/bytesobject.c
PyTypeObject PyBytes_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "bytes",
  .tp_basicsize = PyBytesObject_SIZE,
	.tp_flags = Py_TPFLAGS_BYTES_SUBCLASS,
	.tp_hash = (hashfunc) bytes_hash,
	.tp_free = PyObject_Del,
};

static PyObject *
_PyBytes_FromSize(Py_ssize_t size, int use_calloc) {
	PyBytesObject *op;
	assert(size >= 0);
	
	if (size == 0) {
		assert(false);
	}
	if (use_calloc) {
		assert(false);
	} else {
		op = (PyBytesObject *) PyObject_Malloc(PyBytesObject_SIZE + size);
	}
	assert(op);
	_PyObject_InitVar((PyVarObject*) op, &PyBytes_Type, size);
	op->ob_shash = -1;
	if (!use_calloc) {
		op->ob_sval[size] = '\0';
	}
	return (PyObject *) op;
}

// Return a borrowed reference to the empty bytes string singleton
PyObject *
bytes_get_empty(void) {
	// TODO should use a singleton instead
	PyBytesObject *op = (PyBytesObject *) PyObject_Malloc(PyBytesObject_SIZE);
	assert(op);
	_PyObject_InitVar((PyVarObject*) op, &PyBytes_Type, 0);
	op->ob_shash = -1;
	op->ob_sval[0] = '\0';
	return (PyObject *) op;
}

// Return a strong reference to the empty bytes string singleton
PyObject *
bytes_new_empty(void) {
	PyObject *empty = bytes_get_empty();
	Py_INCREF(empty);
	return (PyObject *) empty;
}

PyObject *
PyBytes_FromStringAndSize(const char *str, Py_ssize_t size) {
	PyBytesObject *op;
	assert(size >= 0);
	if (size == 0) {
		return bytes_new_empty();
	}
	op = (PyBytesObject *) _PyBytes_FromSize(size, 0);
	assert(op);

	if (str == NULL)
		return (PyObject *) op;

	memcpy(op->ob_sval, str, size);
	return (PyObject *) op;
}

char *
PyBytes_AsString(PyObject *op) {
	if (!PyBytes_Check(op)) {
		assert(false);
	}
	return ((PyBytesObject *) op)->ob_sval;
}

int
_PyBytes_Resize(PyObject **pv, Py_ssize_t newsize)
{
	PyObject *v;
	PyBytesObject *sv;
	v = *pv;
	if (!PyBytes_Check(v) || newsize < 0) {
		assert(false);
	}
	if (Py_SIZE(v) == newsize) {
		return 0;
	}
	if (Py_SIZE(v) == 0) {
		assert(false);
	}
	if (Py_REFCNT(v) != 1) {
		assert(false);
	}
	if (newsize == 0) {
		assert(false);
	}

	*pv = (PyObject *)
			PyObject_Realloc(v, PyBytesObject_SIZE + newsize);
	if (*pv == NULL) {
		assert(false);
	}
	_Py_NewReference(*pv);
	sv = (PyBytesObject *) *pv;
	Py_SET_SIZE(sv, newsize);
	sv->ob_sval[newsize] = '\0';
	sv->ob_shash = -1;
	return 0;
}

static Py_hash_t bytes_hash(PyBytesObject *a) {
	if (a->ob_shash == -1) {
		a->ob_shash = _Py_HashBytes(a->ob_sval, Py_SIZE(a));
	}
	return a->ob_shash;
}

#endif
