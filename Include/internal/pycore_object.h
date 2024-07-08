#pragma once

void
_Py_NewReference(PyObject *op) {
	Py_SET_REFCNT(op, 1);
}

void
_PyObject_Init(PyObject *op, PyTypeObject *typeobj) {
	assert(op);
	Py_SET_TYPE(op, typeobj);
	// TODO may need increment the refcount for typeobj
	_Py_NewReference(op);
}

void
_PyObject_InitVar(PyVarObject *op, PyTypeObject *typeobj, Py_ssize_t size) {
	assert(op);
	Py_SET_SIZE(op, size);
	_PyObject_Init((PyObject *) op, typeobj);
}
