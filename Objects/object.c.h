#pragma once

#include "descrobject.h"
#include "pyerrors.h"

int PyErr_GivenExceptionMatches(PyObject *err, PyObject *exc);

int _Py_SwappedOp[] = {
  Py_GT,
  Py_GE,
  Py_EQ,
  Py_NE,
  Py_LT,
  Py_LE,
};

static inline int set_attribute_error_context(PyObject *v, PyObject *name);

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
  //  printf("PyObject_SetAttr for v of type %s\n", Py_TYPE(v)->tp_name);
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
  if (tp->tp_setattr != NULL) {
    assert(false);
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
  descrsetfunc f;
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
    Py_INCREF(descr);
    f = Py_TYPE(descr)->tp_descr_set;
    if (f != NULL) {
      res = f(descr, obj, value);
      goto done;
    }
  }

  if (dict == NULL) {
    dictptr = _PyObject_GetDictPtr(obj);
    if (dictptr == NULL) {
      printf("obj ptr %p, object of type %s has no dict, attr name %s\n", obj, tp->tp_name, (char*) PyUnicode_DATA(name));
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
    printf("PyObject_GetAttr obj type %s, attr name %s\n", Py_TYPE(v)->tp_name, (char *) PyUnicode_DATA(name));
    assert(false);
  }
  if (result == NULL) {
    set_attribute_error_context(v, name);
  }
  return result;
}

int PyErr_ExceptionMatches(PyObject *exc);
void PyErr_NormalizeException(PyObject **exc, PyObject **val, PyObject **tb);

static inline int
set_attribute_error_context(PyObject *v, PyObject *name)
{
  assert(PyErr_Occurred());
  _Py_IDENTIFIER(name);
  _Py_IDENTIFIER(obj);
  if (!PyErr_ExceptionMatches(PyExc_AttributeError)) {
    return 0;
  }
  PyObject *type, *value, *traceback;
  PyErr_Fetch(&type, &value, &traceback);
  PyErr_NormalizeException(&type, &value, &traceback);
  if (!PyErr_GivenExceptionMatches(value, PyExc_AttributeError)) {
    goto restore;
  }
  PyAttributeErrorObject *the_exc = (PyAttributeErrorObject *) value;
  if (the_exc->name || the_exc->obj) {
    goto restore;
  }

  if (_PyObject_SetAttrId(value, &PyId_name, name) ||
      _PyObject_SetAttrId(value, &PyId_obj, v)) {
    return 1;
  }

restore:
  PyErr_Restore(type, value, traceback);
  return 0;
}

PyObject *
_PyObject_GenericGetAttrWithDict(PyObject *obj, PyObject *name,
    PyObject *dict, int suppress)
{
  PyTypeObject *tp = Py_TYPE(obj);
  PyObject *descr = NULL;
  PyObject *res = NULL;
  descrgetfunc f;
  Py_ssize_t dictoffset;
  PyObject **dictptr;

  if (!PyUnicode_Check(name)) {
    assert(false);
  }
  Py_INCREF(name);

  if (tp->tp_dict == NULL) {
    if (PyType_Ready(tp) < 0)
      assert(false);
  }
  // printf("get attr %s from type %s\n", (char*) PyUnicode_DATA(name), tp->tp_name);

  descr = _PyType_Lookup(tp, name);

  f = NULL;
  if (descr != NULL) {
    Py_INCREF(descr);
    f = Py_TYPE(descr)->tp_descr_get;
    // printf("f %p, is data %d\n", f, PyDescr_IsData(descr));
    if (f != NULL && PyDescr_IsData(descr)) {
      res = f(descr, obj, (PyObject *) Py_TYPE(obj));
      if (res == NULL) {
        assert(false);
      }
      goto done;
    }
  }

  if (dict == NULL) {
    dictoffset = tp->tp_dictoffset;
    if (dictoffset != 0) {
      if (dictoffset < 0) {
        assert(false);
      }
      dictptr = (PyObject **) ((char *) obj + dictoffset);
      dict = *dictptr;
    }
  }

  if (dict != NULL) {
    Py_INCREF(dict);
    res = PyDict_GetItemWithError(dict, name);
    if (res != NULL) {
      Py_INCREF(res);
      Py_DECREF(dict);
      goto done;
    } else {
      printf("check '%s' object attribute '%s'\n", tp->tp_name, (char*) PyUnicode_DATA(name));
      Py_DECREF(dict);
      if (PyErr_Occurred()) {
        assert(false);
      }
    }
  }

  if (f != NULL) {
    res = f(descr, obj, (PyObject *) Py_TYPE(obj));
    assert(res);
    goto done;
  }

  if (descr != NULL) {
    res = descr;
    descr = NULL;
    goto done;
  }

  if (!suppress) {
    if (strcmp(tp->tp_name, "module") == 0) {
      printf("'%s' object (str repr %s) has no attribute '%s' - may get fixedup\n", tp->tp_name, (char *) PyUnicode_DATA(PyObject_Str(obj)), (char*) PyUnicode_DATA(name));
    } else {
      printf("'%s' object has no attribute '%s' - may get fixedup\n", tp->tp_name, (char*) PyUnicode_DATA(name));
    }
    // if (strcmp((char *) PyUnicode_DATA(name), "foo") == 0) { fail(0); }
    PyErr_Format(PyExc_AttributeError,
      // "'%.50s' object has no attribute '%U'",
      "'%s' object has no attribute '%U'",
      tp->tp_name, name);

    set_attribute_error_context(obj, name);
    if (strcmp(PyUnicode_DATA(name), "platform") == 0) {
      fail(0);
    }
  }
 done:
  Py_XDECREF(descr);
  Py_DECREF(name);
  return res;
}

PyObject *
PyObject_GenericGetAttr(PyObject *obj, PyObject *name) {
  return _PyObject_GenericGetAttrWithDict(obj, name, NULL, 0);
}

int _PyObject_GetMethod(PyObject *obj, PyObject *name, PyObject **method) {
  PyTypeObject *tp = Py_TYPE(obj);
  PyObject *descr;
  descrgetfunc f = NULL;
  PyObject **dictptr, *dict;
  PyObject *attr;
  int meth_found = 0;

  assert(*method == NULL);

  printf("Py_TYPE(obj)->tp_getattro %p, type %s, name %s\n", Py_TYPE(obj)->tp_getattro, Py_TYPE(obj)->tp_name, (char *) PyUnicode_DATA(name));
  if (Py_TYPE(obj)->tp_getattro != PyObject_GenericGetAttr
      || !PyUnicode_Check(name)) {
    *method = PyObject_GetAttr(obj, name);
    return 0;
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
    Py_INCREF(dict);
    attr = PyDict_GetItemWithError(dict, name);
    if (attr != NULL) {
      Py_INCREF(attr);
      *method = attr;
      Py_DECREF(dict);
      Py_XDECREF(descr);
      return 0;
    } else {
      Py_DECREF(dict);
      if (PyErr_Occurred()) {
        Py_XDECREF(descr);
        return 0;
      }
    }
  }

  if (meth_found) {
    *method = descr;
    return 1;
  }

  if (f != NULL) {
    assert(false);
  }

  if (descr != NULL) {
    assert(false);
  }

  PyErr_Format(PyExc_AttributeError,
      "'%s' object has no attribute '%U'",
      tp->tp_name, name);

  set_attribute_error_context(obj, name);
  return 0;
}

PyObject *
PyObject_Repr(PyObject *v) {
  PyObject *res;
  assert(v != NULL);
  if (Py_TYPE(v)->tp_repr == NULL) {
    printf("type %s has no tp_repr defined\n", Py_TYPE(v)->tp_name);
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
    if (PyUnicode_READY(v) < 0) {
      return NULL;
    }
    Py_INCREF(v);
    return v;
  }
  if (Py_TYPE(v)->tp_str == NULL) {
    return PyObject_Repr(v);
  }

  res = (*Py_TYPE(v)->tp_str)(v);
  if (res == NULL) {
    return NULL;
  }
  if (!PyUnicode_Check(res)) {
    fail(0);
  }
  if (PyUnicode_READY(res) < 0) {
    return NULL;
  }
  return res;
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
  } else if (Py_TYPE(v)->tp_as_sequence != NULL &&
      Py_TYPE(v)->tp_as_sequence->sq_length != NULL) {
    res = (*Py_TYPE(v)->tp_as_sequence->sq_length)(v);
	} else {
		assert(false);
	}
	return (res > 0) ? 1 : Py_SAFE_DOWNCAST(res, Py_ssize_t, int);
}

void PyErr_Clear(void);
int _PyObject_LookupAttr(PyObject *v, PyObject *name, PyObject **result) {
  PyTypeObject *tp = Py_TYPE(v);

  if (!PyUnicode_Check(name)) {
    assert(false);
  }

  if (tp->tp_getattro == PyObject_GenericGetAttr) {
    *result = _PyObject_GenericGetAttrWithDict(v, name, NULL, 1);
    if (*result != NULL) {
      return 1;
    }
    if (PyErr_Occurred()) {
      return -1;
    }
    return 0;
  }
  if (tp->tp_getattro != NULL) {
    *result = (*tp->tp_getattro)(v, name);
  } else if (tp->tp_getattr != NULL) {
    assert(false);
  } else {
    assert(false);
    *result = NULL;
    return 0;
  }

  if (*result != NULL) {
    return 1;
  }
  if (!PyErr_ExceptionMatches(PyExc_AttributeError)) {
    return -1;
  }
  PyErr_Clear();
  return 0;
}

int _PyObject_LookupAttrId(PyObject *v, struct _Py_Identifier *name, PyObject **result) {
  PyObject *oname = _PyUnicode_FromId(name);
  if (!oname) {
    assert(false);
  }
  return _PyObject_LookupAttr(v, oname, result);
  assert(false);
}

PyObject *
_PyObject_GetAttrId(PyObject *v, _Py_Identifier *name) {
  PyObject *result;
  PyObject *oname = _PyUnicode_FromId(name);
  if (!oname)
    return NULL;
  result = PyObject_GetAttr(v, oname);
  return result;
}

void PyObject_CallFinalizer(PyObject *self) {
  PyTypeObject *tp = Py_TYPE(self);

  if (tp->tp_finalize == NULL)
    return;

  // TODO follow cpy
  #if 0
  if (_PyType_IS_GC(tp) && _PyGC_FINALIZED(self))
    return;
  #endif

  tp->tp_finalize(self);

  // TODO follow cpy
  #if 0
  if (_PyType_IS_GC(tp)) {
    _PyGC_SET_FINALIZED(self);
  }
  #endif
}

int PyObject_CallFinalizerFromDealloc(PyObject *self) {
  if (Py_REFCNT(self) != 0) {
    assert(false);
  }

  // temporarily resurrect the object
  Py_SET_REFCNT(self, 1);

  PyObject_CallFinalizer(self);

  assert(Py_REFCNT(self) > 0);
  Py_SET_REFCNT(self, Py_REFCNT(self) - 1);
  if (Py_REFCNT(self) == 0) {
    return 0;
  }
  assert(false);
}

int PyObject_GenericSetDict(PyObject *obj, PyObject *value, void *context) {
  assert(false);
}

PyObject *
PyObject_GetAttrString(PyObject *v, const char *name) {
  PyObject *w, *res;

  if (Py_TYPE(v)->tp_getattr != NULL)
    return (*Py_TYPE(v)->tp_getattr)(v, (char *) name);
  w = PyUnicode_FromString(name);
  if (w == NULL)
    return NULL;
  res = PyObject_GetAttr(v, w);
  Py_DECREF(w);
  return res;
}

PyObject *PyObject_Bytes(PyObject *v) {
  if (v == NULL) {
    fail(0);
  }

  if (PyBytes_CheckExact(v)) {
    Py_INCREF(v);
    return v;
  }
  fail(0);
}

int PyObject_HasAttr(PyObject *v, PyObject *name) {
  PyObject *res;
  if (_PyObject_LookupAttr(v, name, &res) < 0) {
    PyErr_Clear();
    return 0;
  }
  if (res == NULL)
    return 0;
  Py_DECREF(res);
  return 1;
}
