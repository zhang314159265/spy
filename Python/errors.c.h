#pragma once

void
_PyErr_Restore(PyThreadState *tstate, PyObject *type, PyObject *value,
    PyObject *traceback) {
  PyObject *oldtype, *oldvalue, *oldtraceback;

  if (traceback != NULL) {
    assert(false);
  }

  oldtype = tstate->curexc_type;
  oldvalue = tstate->curexc_value;
  oldtraceback = tstate->curexc_traceback;

  tstate->curexc_type = type;
  tstate->curexc_value = value;
  tstate->curexc_traceback = traceback;

  Py_XDECREF(oldtype);
  Py_XDECREF(oldvalue);
  Py_XDECREF(oldtraceback);
}

void
_PyErr_Clear(PyThreadState *tstate)
{
  _PyErr_Restore(tstate, NULL, NULL, NULL);
}

void PyErr_Clear(void) {
  PyThreadState *tstate = _PyThreadState_GET();
  _PyErr_Clear(tstate);
}

_PyErr_StackItem *
_PyErr_GetTopmostException(PyThreadState *tstate) {
  _PyErr_StackItem *exc_info = tstate->exc_info;
  while ((exc_info->exc_type == NULL || exc_info->exc_type == Py_None) &&
      exc_info->previous_item != NULL)
  {
    exc_info = exc_info->previous_item;
  }
  return exc_info;
}

void
_PyErr_SetObject(PyThreadState *tstate, PyObject *exception, PyObject *value) {
  PyObject *exc_value;
  PyObject *tb = NULL;

  if (exception != NULL &&
      !PyExceptionClass_Check(exception)) {
    assert(false);
  }
  Py_XINCREF(value);
  exc_value = _PyErr_GetTopmostException(tstate)->exc_value;
  if (exc_value != NULL && exc_value != Py_None) {
    assert(false);
  }
  if (value != NULL && PyExceptionInstance_Check(value)) {
    assert(false);
  }
  Py_XINCREF(exception);
  _PyErr_Restore(tstate, exception, value, tb);
}

static PyObject *
_PyErr_FormatV(PyThreadState *tstate, PyObject *exception,
    const char *format, va_list vargs)
{
  PyObject *string;
  _PyErr_Clear(tstate);

  string = PyUnicode_FromFormatV(format, vargs);

  _PyErr_SetObject(tstate, exception, string);
  Py_XDECREF(string);
  return NULL;
}

PyObject *
PyErr_Format(PyObject *exception, const char *format, ...) {
  PyThreadState *tstate = _PyThreadState_GET();
  va_list vargs;
  va_start(vargs, format);
  _PyErr_FormatV(tstate, exception, format, vargs);
  va_end(vargs);
  return NULL;
}

int PyErr_GivenExceptionMatches(PyObject *err, PyObject *exc) {
  if (err == NULL || exc == NULL) {
    return 0;
  }
  if (PyTuple_Check(exc)) {
    assert(false);
  }
  if (PyExceptionInstance_Check(err)) {
    err = PyExceptionInstance_Class(err);
  }
  if (PyExceptionClass_Check(err) && PyExceptionClass_Check(exc)) {
    return PyType_IsSubtype((PyTypeObject *) err, (PyTypeObject *) exc);
  }

  return err == exc;
}

int
_PyErr_ExceptionMatches(PyThreadState *tstate, PyObject *exc) {
  return PyErr_GivenExceptionMatches(_PyErr_Occurred(tstate), exc);
}

int PyErr_ExceptionMatches(PyObject *exc) {
  PyThreadState *tstate = _PyThreadState_GET();
  return _PyErr_ExceptionMatches(tstate, exc);
}

void
_PyErr_Fetch(PyThreadState *tstate, PyObject **p_type, PyObject **p_value,
    PyObject **p_traceback) {
  *p_type = tstate->curexc_type;
  *p_value = tstate->curexc_value;
  *p_traceback = tstate->curexc_traceback;

  tstate->curexc_type = NULL;
  tstate->curexc_value = NULL;
  tstate->curexc_traceback = NULL;
}

void PyErr_Fetch(PyObject **p_type, PyObject **p_value, PyObject **p_traceback) {
  PyThreadState *tstate = _PyThreadState_GET();
  _PyErr_Fetch(tstate, p_type, p_value, p_traceback);
}

static PyObject *
_PyErr_CreateException(PyObject *exception_type, PyObject *value) {
  PyObject *exc;

  if (value == NULL || value == Py_None) {
    assert(false);
  } else if (PyTuple_Check(value)) {
    assert(false);
  } else {
    exc = PyObject_CallOneArg(exception_type, value);
  }

  if (exc != NULL && !PyExceptionInstance_Check(exc)) {
    assert(false);
  }
  return exc;
}

void
_PyErr_NormalizeException(PyThreadState *tstate, PyObject **exc,
    PyObject **val, PyObject **tb)
{
  PyObject *type, *value, *internal_tb;
 restart:
  type = *exc;
  if (type == NULL) {
    return;
  }

  value = *val;
  if (!value) {
    value = Py_None;
    Py_INCREF(value);
  }

  if (PyExceptionClass_Check(type)) {
    PyObject *inclass = NULL;
    int is_subclass = 0;

    if (PyExceptionInstance_Check(value)) {
      inclass = PyExceptionInstance_Class(value);
      is_subclass = PyObject_IsSubclass(inclass, type);
      if (is_subclass < 0) {
        assert(false);
      }
    }
    if (!is_subclass) {
      PyObject *fixed_value = _PyErr_CreateException(type, value);
      if (fixed_value == NULL) {
        assert(false);
      }
      Py_DECREF(value);
      value = fixed_value;
    } else if (inclass != type) {
      assert(false);
    }
  }
  *exc = type;
  *val = value;
  return;
  assert(false);
}

void PyErr_NormalizeException(PyObject **exc, PyObject **val, PyObject **tb) {
  PyThreadState *tstate = _PyThreadState_GET();
  _PyErr_NormalizeException(tstate, exc, val, tb);
}

void PyErr_Restore(PyObject *type, PyObject *value, PyObject *traceback) {
  PyThreadState *tstate = _PyThreadState_GET();
  _PyErr_Restore(tstate, type, value, traceback);
}
