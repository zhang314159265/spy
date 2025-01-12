#pragma once

#include <math.h>
#include "pymath.h"

typedef struct {
	PyObject_HEAD
	double ob_fval;
} PyFloatObject;

static Py_hash_t float_hash(PyFloatObject *v);
static PyObject *float_floor_div(PyObject *v, PyObject *w);
static PyObject *float_div(PyObject *v, PyObject *w);
static PyObject *float_add(PyObject *v, PyObject *w);
static void float_dealloc(PyFloatObject *op);
static PyObject *float_sub(PyObject *v, PyObject *w);
static PyObject *float_abs(PyFloatObject *v);
static PyObject *float_richcompare(PyObject *v, PyObject *w, int op);
static PyObject *float_repr(PyFloatObject *v);
static PyObject * float___trunc___impl(PyObject *self);

static PyNumberMethods float_as_number = {
	.nb_floor_divide = float_floor_div,
	.nb_true_divide = float_div,
	.nb_add = float_add,
	.nb_subtract = float_sub,
	.nb_absolute = (unaryfunc) float_abs,
  .nb_int = float___trunc___impl,
};

PyTypeObject PyFloat_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "float",
	.tp_basicsize = sizeof(PyFloatObject),
	.tp_hash = (hashfunc) float_hash,
	.tp_as_number = &float_as_number,
	.tp_dealloc = (destructor) float_dealloc,
	.tp_richcompare = float_richcompare,
	.tp_str = 0,
	.tp_repr = (reprfunc) float_repr,
};

#define PyFloat_Check(op) PyObject_TypeCheck(op, &PyFloat_Type)
#define PyFloat_CheckExact(op) Py_IS_TYPE(op, &PyFloat_Type)

// defined cpy/Objects/floatobject.c
PyObject *
PyFloat_FromDouble(double fval) {
	PyFloatObject *op = PyObject_Malloc(sizeof(PyFloatObject));
	if (!op) {
		assert(false);
	}
	_PyObject_Init((PyObject *) op, &PyFloat_Type);
	op->ob_fval = fval;
	return (PyObject *) op;
}

#define PyFloat_AS_DOUBLE(op) (((PyFloatObject *) (op))->ob_fval)

double
PyFloat_AsDouble(PyObject *op) {
	if (op == NULL) {
		assert(false);
	}

	if (PyFloat_Check(op)) {
		return PyFloat_AS_DOUBLE(op);	
	}
	assert(false);
}

static Py_hash_t float_hash(PyFloatObject *v) {
	return _Py_HashDouble((PyObject *) v, v->ob_fval);
}

#define CONVERT_TO_DOUBLE(obj, dbl) \
	if (PyFloat_Check(obj)) \
		dbl = PyFloat_AS_DOUBLE(obj); \
	else if (convert_to_double(&(obj), &(dbl)) < 0) \
		return obj;

static int convert_to_double(PyObject **v, double *dbl);

static PyObject *float_floor_div(PyObject *v, PyObject *w) {
	assert(false);
}

static PyObject *float_div(PyObject *v, PyObject *w) {
	double a, b;
	CONVERT_TO_DOUBLE(v, a);
	CONVERT_TO_DOUBLE(w, b);
	if (b == 0.0) {
		assert(false);
	}
	a = a / b;
	return PyFloat_FromDouble(a);
}

static PyObject *float_add(PyObject *v, PyObject *w) {
	double a, b;
	CONVERT_TO_DOUBLE(v, a);
	CONVERT_TO_DOUBLE(w, b);
	a = a + b;
	return PyFloat_FromDouble(a);
}

static void float_dealloc(PyFloatObject *op) {
	if (PyFloat_CheckExact(op)) {
		PyObject_Free(op);
		return;
	} else {
		assert(false);
	}
}

static PyObject *float_sub(PyObject *v, PyObject *w) {
	double a, b;
	CONVERT_TO_DOUBLE(v, a);
	CONVERT_TO_DOUBLE(w, b);
	a = a - b;
	return PyFloat_FromDouble(a);
}

static PyObject *float_abs(PyFloatObject *v) {
	return PyFloat_FromDouble(fabs(v->ob_fval));
}

static PyObject *float_richcompare(PyObject *v, PyObject *w, int op);

int _PyFloat_FormatAdvancedWriter(_PyUnicodeWriter *writer,
		PyObject *obj, PyObject *format_spec, Py_ssize_t start, Py_ssize_t end);

static PyObject *float_repr(PyFloatObject *v) {
	// TODO: follow cpython
	PyObject *result;
	char buf[256];
	snprintf(buf, 256, "%lf", PyFloat_AS_DOUBLE(v));
	result = _PyUnicode_FromASCII(buf, strlen(buf));
	return result;
}
