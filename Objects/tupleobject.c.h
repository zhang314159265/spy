#pragma once

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
