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

int
PyObject_SetAttr(PyObject *v, PyObject *name, PyObject *value) {
  PyTypeObject *tp = Py_TYPE(v);
  int err;

  if (!PyUnicode_Check(name)) {
    assert(false);
  }
  Py_INCREF(name);

  PyUnicode_InternInPlace(&name);
  if (tp->tp_setattro != NULL) {
    err = (*tp->tp_setattro)(v, name, value); 
    Py_DECREF(name);
    return err;
  }
  assert(false);
}

int
PyObject_SetAttrString(PyObject *v, const char *name, PyObject *w) {
  PyObject *s;
  int res;

  // printf("PyObject_SetAttrString type for v %s\n", Py_TYPE(v)->tp_name);

  if (Py_TYPE(v)->tp_setattr != NULL) {
    assert(false);
  }
  s = PyUnicode_InternFromString(name);
  if (s == NULL)
    return -1;
  res = PyObject_SetAttr(v, s, w);
  Py_XDECREF(s);
  return res;
}

PyObject **
_PyObject_GetDictPtr(PyObject *obj) {
  Py_ssize_t dictoffset;
  PyTypeObject *tp = Py_TYPE(obj);

  dictoffset = tp->tp_dictoffset;
  if (dictoffset == 0) {
    assert(false);
  }
  if (dictoffset < 0) {
    assert(false);
  }
  return (PyObject **) ((char *)obj + dictoffset);
}

int
_PyObject_GenericSetAttrWithDict(PyObject *obj, PyObject *name,
    PyObject *value, PyObject *dict) {

  PyTypeObject *tp = Py_TYPE(obj);
  PyObject *descr;
  PyObject **dictptr;
  int res = -1;

  if (!PyUnicode_Check(name)) {
    assert(false);
  }

  if (tp->tp_dict == NULL && PyType_Ready(tp) < 0)
    return -1;

  Py_INCREF(name);

  descr = _PyType_Lookup(tp, name);

  if (descr != NULL) {
    assert(false);
  }

  if (dict == NULL) {
    dictptr = _PyObject_GetDictPtr(obj);
    if (dictptr == NULL) {
      assert(false);
    }
    res = _PyObjectDict_SetItem(tp, dictptr, name, value);
  } else {
    assert(false);
  }
  if (res < 0) {
    assert(false);
  }
 done:
  Py_XDECREF(descr);
  Py_DECREF(name);
  return res;
}

int PyObject_GenericSetAttr(PyObject *obj, PyObject *name, PyObject *value) {
  return _PyObject_GenericSetAttrWithDict(obj, name, value, NULL);
}

int
_PyObject_SetAttrId(PyObject *v, _Py_Identifier *name, PyObject *w)
{
  int result;
  PyObject *oname = _PyUnicode_FromId(name);
  if (!oname)
    return -1;
  result = PyObject_SetAttr(v, oname, w);
  return result;
}
