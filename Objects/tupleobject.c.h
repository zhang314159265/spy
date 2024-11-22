#pragma once

#include "Python/getargs.c.h"

static PyObject *tuplerichcompare(PyObject *v, PyObject *w, int op) {
	PyTupleObject *vt, *wt;
	Py_ssize_t i;
	Py_ssize_t vlen, wlen;

	if (!PyTuple_Check(v) || !PyTuple_Check(w))
		Py_RETURN_NOTIMPLEMENTED;

	vt = (PyTupleObject *) v;
	wt = (PyTupleObject *) w;

	vlen = Py_SIZE(vt);
	wlen = Py_SIZE(wt);

	// Search for the first index where items are different.
	for (i = 0; i < vlen && i < wlen; i++) {
		int k = PyObject_RichCompareBool(vt->ob_item[i],
			wt->ob_item[i], Py_EQ);
		if (k < 0)
			return NULL;
		if (!k)
			break;
	}

	if (i >= vlen || i >= wlen) {
		Py_RETURN_RICHCOMPARE(vlen, wlen, op);
	}

	if (op == Py_EQ) {
		Py_RETURN_FALSE;
	}
	if (op == Py_NE) {
		Py_RETURN_TRUE;
	}

	return PyObject_RichCompare(vt->ob_item[i], wt->ob_item[i], op);
}

static PyObject *
tupleslice(PyTupleObject *a, Py_ssize_t ilow,
    Py_ssize_t ihigh) {
  if (ilow < 0)
    ilow = 0;
  if (ihigh > Py_SIZE(a))
    ihigh = Py_SIZE(a);
  if (ihigh < ilow)
    ihigh = ilow;
  if (ilow == 0 && ihigh == Py_SIZE(a) && PyTuple_CheckExact(a)) {
    Py_INCREF(a);
    return (PyObject *) a;
  }
  return _PyTuple_FromArray(a->ob_item + ilow, ihigh - ilow);
}

PyObject *PyTuple_GetSlice(PyObject *op, Py_ssize_t i, Py_ssize_t j) {
  if (op == NULL || !PyTuple_Check(op)) {
    assert(false);
  }
  return tupleslice((PyTupleObject *) op, i, j);
}

static PyObject *
tuple_new_impl(PyTypeObject *type, PyObject *iterable) {
  printf("tuple_new_impl iterable type %s\n", Py_TYPE(iterable)->tp_name);

  if (type != &PyTuple_Type) {
    assert(false);
  }

  if (iterable == NULL) {
    assert(false);
  } else {
    return PySequence_Tuple(iterable);
  }
}

// defined in cpy/Objects/clinic/tupleobject.c.h
static PyObject *tuple_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
  PyObject *return_value = NULL;
  PyObject *iterable = NULL;

  if ((type == &PyTuple_Type) &&
      !_PyArg_NoKeywords("tuple", kwargs)) {
    goto exit;
  }
  if (!_PyArg_CheckPositional("tuple", PyTuple_GET_SIZE(args), 0, 1)) {
    goto exit;
  }
  if (PyTuple_GET_SIZE(args) < 1) {
    goto skip_optional;
  }
  iterable = PyTuple_GET_ITEM(args, 0);
 skip_optional:
  return_value = tuple_new_impl(type, iterable);
 exit:
  return return_value;
}

int _PyTuple_Resize(PyObject **pv, Py_ssize_t newsize) {
  assert(false);
}
