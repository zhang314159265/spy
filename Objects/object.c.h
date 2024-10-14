#pragma once

int _Py_SwappedOp[] = {
  Py_GT,
  Py_GE,
  Py_EQ,
  Py_NE,
  Py_LT,
  Py_LE,
};

static PyObject *
do_richcompare(PyThreadState *tstate, PyObject *v, PyObject *w, int op) {
	richcmpfunc f;
	PyObject *res;
  int checked_reverse_op = 0;

	if (!Py_IS_TYPE(v, Py_TYPE(w)) &&
      PyType_IsSubtype(Py_TYPE(w), Py_TYPE(v)) &&
      (f = Py_TYPE(w)->tp_richcompare) != NULL) {
    printf("v type %s, w type %s\n", Py_TYPE(v)->tp_name, Py_TYPE(w)->tp_name);
		assert(false);
	}
	if ((f = Py_TYPE(v)->tp_richcompare) != NULL) {
		res = (*f)(v, w, op);
		if (res != Py_NotImplemented)
			return res;
		Py_DECREF(res);
	}

  if (!checked_reverse_op && (f = Py_TYPE(w)->tp_richcompare) != NULL) {
    res = (*f)(w, v, _Py_SwappedOp[op]);
    if (res != Py_NotImplemented)
      return res;
     Py_DECREF(res);
  }

  switch (op) {
  case Py_EQ:
    res = (v == w) ? Py_True : Py_False;
    break;
  case Py_NE:
    assert(false);
  default:
	  printf("Fail to do richcompare for object of type %s\n", Py_TYPE(v)->tp_name);
    assert(false);
  }

  Py_INCREF(res);
  return res;
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
    return NULL;
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

PyObject *PyObject_GetAttr(PyObject *v, PyObject *name) {
  PyTypeObject *tp = Py_TYPE(v);
  if (!PyUnicode_Check(name)) {
    assert(false);
  }

  PyObject *result = NULL;
  if (tp->tp_getattro != NULL) {
    result = (*tp->tp_getattro)(v, name);
  } else if (tp->tp_getattr != NULL) {
    assert(false);
  } else {
    assert(false);
  }
  if (result == NULL) {
    assert(false);
  }
  return result;
}

PyObject *
_PyObject_GenericGetAttrWithDict(PyObject *obj, PyObject *name,
    PyObject *dict, int suppress)
{
  PyTypeObject *tp = Py_TYPE(obj);
  PyObject *descr = NULL;

  if (!PyUnicode_Check(name)) {
    assert(false);
  }
  Py_INCREF(name);

  if (tp->tp_dict == NULL) {
    if (PyType_Ready(tp) < 0)
      assert(false);
  }

  descr = _PyType_Lookup(tp, name);
  assert(false);
}

PyObject *
PyObject_GenericGetAttr(PyObject *obj, PyObject *name) {
  return _PyObject_GenericGetAttrWithDict(obj, name, NULL, 0);
}

int _PyObject_GetMethod(PyObject *obj, PyObject *name, PyObject **method) {
  PyTypeObject *tp = Py_TYPE(obj);
  PyObject *descr;
  PyObject **dictptr, *dict;
  int meth_found = 0;

  assert(*method == NULL);

  if (Py_TYPE(obj)->tp_getattro != PyObject_GenericGetAttr
      || !PyUnicode_Check(name)) {
    assert(false);
  }

  if (tp->tp_dict == NULL && PyType_Ready(tp) < 0) {
    return 0;
  }

  descr = _PyType_Lookup(tp, name);
  if (descr != NULL) {
    Py_INCREF(descr);
    if (_PyType_HasFeature(Py_TYPE(descr), Py_TPFLAGS_METHOD_DESCRIPTOR)) {
      meth_found = 1;
    } else {
      assert(false);
    }
  }

  dictptr = _PyObject_GetDictPtr(obj);
  if (dictptr != NULL && (dict = *dictptr) != NULL) {
    assert(false);
  }

  if (meth_found) {
    *method = descr;
    return 1;
  }
  assert(false);
}

PyObject *
PyObject_Repr(PyObject *v) {
  PyObject *res;
  assert(v != NULL);
  if (Py_TYPE(v)->tp_repr == NULL) {
    assert(false);
  }
  res = (*Py_TYPE(v)->tp_repr)(v);

  if (res == NULL) {
    return NULL;
  }
  if (!PyUnicode_Check(res)) {
    assert(false);
  }
  return res;
}

PyObject *PyObject_Str(PyObject *v) {
  PyObject *res;
  assert(v != NULL);

  if (PyUnicode_CheckExact(v)) {
    assert(false);
  }
  if (Py_TYPE(v)->tp_str == NULL) {
    return PyObject_Repr(v);
  }
  assert(false);
}

// 1 for value interpreted as true, 0 for value interpreted as false;
// -1 for error
int PyObject_IsTrue(PyObject *v) {
	Py_ssize_t res;
	if (v == Py_True)
		return 1;
	if (v == Py_False)
		return 0;
	if (v == Py_None)
		return 0;
	else if (Py_TYPE(v)->tp_as_number != NULL &&
			Py_TYPE(v)->tp_as_number->nb_bool != NULL) {
		res = (*Py_TYPE(v)->tp_as_number->nb_bool)(v);
	} else {
		assert(false);
	}
	return (res > 0) ? 1 : Py_SAFE_DOWNCAST(res, Py_ssize_t, int);
}


