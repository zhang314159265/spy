#pragma once

static int convert_to_double(PyObject **v, double *dbl) {
	PyObject *obj = *v;

	if (PyLong_Check(obj)) {
		*dbl = PyLong_AsDouble(obj);
		if (*dbl == -1.0 && PyErr_Occurred()) {
			*v = NULL;
			return -1;
		}
	} else {
		assert(false);
	}
	return 0;
}

static PyObject *float_richcompare(PyObject *v, PyObject *w, int op) {
	double i, j;
	int r = 0;

	assert(PyFloat_Check(v));
	i = PyFloat_AS_DOUBLE(v);

	if (PyFloat_Check(w)) {
		j = PyFloat_AS_DOUBLE(w);
  } else if (!Py_IS_FINITE(i)) {
    fail(0);
  } else if (PyLong_Check(w)) {
    int vsign = i == 0.0 ? 0 : i < 0.0 ? -1 : 1;
    int wsign = _PyLong_Sign(w);

    if (vsign != wsign) {
      i = (double) vsign;
      j = (double) wsign;
      goto Compare;
    }
    fail(0);
	} else {
		assert(false);
	}
Compare:
	switch (op) {
  case Py_EQ:
    r = i == j;
    break;
  case Py_NE:
    r = i != j;
    break;
	case Py_LT:
		r = i < j;
		break;
	default:
    fail("float_richcompare unhandled op %d\n", op);
	}
	return PyBool_FromLong(r);
}

static PyObject * float___trunc___impl(PyObject *self) {
  return PyLong_FromDouble(PyFloat_AS_DOUBLE(self));
}

