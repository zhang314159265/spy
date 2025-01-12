#pragma once

PyObject *_PyNumber_Index(PyObject *item) {
	if (item == NULL) {
		assert(false);
	}

	if (PyLong_Check(item)) {
		Py_INCREF(item);
		return item;
	}
  if (!_PyIndex_Check(item)) {
    PyErr_Format(PyExc_TypeError,
        "'%s' object cannot be interpreted "
        "as an integer", Py_TYPE(item)->tp_name);
    return NULL;
  }

  PyObject *result = Py_TYPE(item)->tp_as_number->nb_index(item);
  assert(result != NULL);
  if (!result || PyLong_CheckExact(result)) {
    return result;
  }
	assert(false);
}

static inline PyObject *_PyObject_VectorcallMethodId(_Py_Identifier *name, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
  PyObject *oname = _PyUnicode_FromId(name);
  if (!oname)
    return NULL;
  return PyObject_VectorcallMethod(oname, args, nargsf, kwnames);
}

static inline PyObject *_PyObject_CallMethodIdNoArgs(PyObject *self, _Py_Identifier *name) {
  return _PyObject_VectorcallMethodId(name, &self,
      1 | PY_VECTORCALL_ARGUMENTS_OFFSET, NULL);
}


PyObject *PyObject_VectorcallMethod(PyObject *name, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
  assert(name != NULL);
  assert(args != NULL);
  assert(PyVectorcall_NARGS(nargsf) >= 1);

  PyThreadState *tstate = _PyThreadState_GET();
  PyObject *callable = NULL;

  // Use args[0] as "self" argument
  int unbound = _PyObject_GetMethod(args[0], name, &callable);
  if (callable == NULL) {
    return NULL;
  }

  if (unbound) {
    nargsf &= ~PY_VECTORCALL_ARGUMENTS_OFFSET;
  } else {
    // Skip "self"
    args++;
    nargsf--;
  }

  PyObject *result = _PyObject_VectorcallTstate(tstate, callable,
      args, nargsf, kwnames);
  Py_DECREF(callable);
  return result;
}
