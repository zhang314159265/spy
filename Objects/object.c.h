#pragma once

static PyObject *
do_richcompare(PyThreadState *tstate, PyObject *v, PyObject *w, int op) {
	richcmpfunc f;
	PyObject *res;

	if (!Py_IS_TYPE(v, Py_TYPE(w))) {
		assert(false);
	}
	if ((f = Py_TYPE(v)->tp_richcompare) != NULL) {
		res = (*f)(v, w, op);
		if (res != Py_NotImplemented)
			return res;
		Py_DECREF(res);
	}

	printf("Fail to do richcompare for object of type %s\n", Py_TYPE(v)->tp_name);
	assert(false);
}

PyObject * PyObject_RichCompare(PyObject *v, PyObject *w, int op) {
  PyThreadState *tstate = _PyThreadState_GET();
  if (v == NULL || w == NULL) {
    assert(false);
  }
  PyObject *res = do_richcompare(tstate, v, w, op);
  return res;
}

// return -1 for error, 0 for false, 1 for true
int PyObject_RichCompareBool(PyObject *v, PyObject *w, int op) {
  PyObject *res;
  int ok;

  if (v == w) {
    if (op == Py_EQ)
      return 1;
    else if (op == Py_NE)
      return 0;
  }

  res = PyObject_RichCompare(v, w, op);
  if (res == NULL)
    return -1;
  if (PyBool_Check(res))
    ok = (res == Py_True);
  else
    assert(false);
  Py_DECREF(res);
  return ok;
}


