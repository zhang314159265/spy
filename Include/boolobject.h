#pragma once

// #include "longobject.h"
#include "longintrepr.h"

#define Py_False ((PyObject *) &_Py_FalseStruct)
#define Py_True ((PyObject *) &_Py_TrueStruct)

#define Py_RETURN_TRUE return Py_NewRef(Py_True)
#define Py_RETURN_FALSE return Py_NewRef(Py_False)

static PyObject *bool_repr(PyObject *self);

PyTypeObject PyBool_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "bool",
	.tp_basicsize = sizeof(struct _longobject),
  .tp_repr = bool_repr,
  .tp_base = &PyLong_Type,
};

struct _longobject _Py_FalseStruct = {
	PyVarObject_HEAD_INIT(&PyBool_Type, 0)
	{ 0}
};


struct _longobject _Py_TrueStruct = {
	PyVarObject_HEAD_INIT(&PyBool_Type, 1)
	{ 1}
};

#define PyBool_Check(x) Py_IS_TYPE(x, &PyBool_Type)

PyObject *PyBool_FromLong(long ok) {
  PyObject *result;

  if (ok)
    result = Py_True;
  else
    result = Py_False;
  Py_INCREF(result);
  return result;
}

#define Py_IsTrue(x) Py_Is((x), Py_True)
#define Py_IsFalse(x) Py_Is((x), Py_False)

static PyObject *false_str = NULL;
static PyObject *true_str = NULL;

static PyObject *bool_repr(PyObject *self) {
  PyObject *s;

  if (self == Py_True)
    s = true_str ? true_str :
      (true_str = PyUnicode_InternFromString("True"));
  else
    s = false_str ? false_str :
      (false_str = PyUnicode_InternFromString("False"));
  Py_XINCREF(s);
  return s;
}
