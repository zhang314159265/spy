#ifndef Py_OBJECT_H
#define Py_OBJECT_H

#include "pyport.h"

#define Py_TPFLAGS_BYTES_SUBCLASS (1UL << 27)
#define Py_TPFLAGS_UNICODE_SUBCLASS (1UL << 28)

typedef struct _typeobject PyTypeObject;

typedef struct _object {
	Py_ssize_t ob_refcnt;
	PyTypeObject *ob_type;
} PyObject;

typedef struct {
	PyObject ob_base;
	Py_ssize_t ob_size;
} PyVarObject;

void _Py_SET_SIZE(PyVarObject *ob, Py_ssize_t size) {
	ob->ob_size = size;
}

#define Py_SET_SIZE(ob, size) _Py_SET_SIZE((PyVarObject*) (ob), size)

void
_Py_SET_TYPE(PyObject *ob, PyTypeObject *type) {
	ob->ob_type = type;
}

#define Py_SET_TYPE(ob, type) _Py_SET_TYPE((PyObject*) ob, type);

void _Py_SET_REFCNT(PyObject *ob, Py_ssize_t refcnt) {
	ob->ob_refcnt = refcnt;
}

#define Py_SET_REFCNT(ob, refcnt) _Py_SET_REFCNT((PyObject *) ob, refcnt)

#define Py_TYPE(ob) (((PyObject *) (ob))->ob_type)

#define PyObject_HEAD PyObject ob_base;
#define PyObject_VAR_HEAD PyVarObject ob_base;

#define PyObject_HEAD_INIT(type)  \
	{ 1, type},

#define PyVarObject_HEAD_INIT(type, size) \
	{ PyObject_HEAD_INIT(type) size },

PyTypeObject PyType_Type; // built-in 'type'

void _Py_INCREF(PyObject *op) {
	op->ob_refcnt++;
}

int
_Py_IS_TYPE(const PyObject *ob, const PyTypeObject *type) {
	return ob->ob_type == type;
}
#define Py_IS_TYPE(ob, type) _Py_IS_TYPE((const PyObject *) (ob), type)

#define Py_INCREF(op) _Py_INCREF((PyObject*) (op))

#include "cpython/object.h"

int
PyType_HasFeature(PyTypeObject *type, unsigned long feature) {
	return ((type->tp_flags & feature) != 0);
}

#define PyType_FastSubclass(type, flag) PyType_HasFeature(type, flag)

#endif
