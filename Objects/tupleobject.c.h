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
