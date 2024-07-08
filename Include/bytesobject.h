#ifndef Py_BYTESOBJECT_H
#define Py_BYTESOBJECT_H

#include <stddef.h> // for offsetof
#include "cpython/bytesobject.h"
#include "objimpl.h"
#include "internal/pycore_object.h"

#define PyBytesObject_SIZE (offsetof(PyBytesObject, ob_sval) + 1)

#define PyBytes_Check(op) \
	PyType_FastSubclass(Py_TYPE(op), Py_TPFLAGS_BYTES_SUBCLASS)

PyTypeObject PyBytes_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "bytes",
	.tp_flags = Py_TPFLAGS_BYTES_SUBCLASS,
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
	assert(str);
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

#endif
