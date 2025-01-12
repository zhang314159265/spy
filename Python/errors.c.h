#pragma once

void
_PyErr_Restore(PyThreadState *tstate, PyObject *type, PyObject *value,
    PyObject *traceback) {
  PyObject *oldtype, *oldvalue, *oldtraceback;

  if (traceback != NULL && !PyTraceBack_Check(traceback)) {
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

static PyObject *_PyErr_CreateException(PyObject *exception_type, PyObject *value);

PyObject *PyException_GetContext(PyObject *self) {
  PyObject *context = _PyBaseExceptionObject_cast(self)->context;
  Py_XINCREF(context);
  return context;
}

void PyException_SetContext(PyObject *self, PyObject *context) {
  Py_XSETREF(_PyBaseExceptionObject_cast(self)->context, context);
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
    Py_INCREF(exc_value);
    if (value == NULL || !PyExceptionInstance_Check(value)) {
      PyObject *fixed_value;

      _PyErr_Clear(tstate);

      fixed_value = _PyErr_CreateException(exception, value);
      Py_XDECREF(value);
      if (fixed_value == NULL) {
        Py_DECREF(exc_value);
        return;
      }

      value = fixed_value;
    }
    if (exc_value != value) {
      PyObject *o = exc_value, *context;
      PyObject *slow_o = o; // Floyd's cycle detection algo
      int slow_update_toggle = 0;
      while ((context = PyException_GetContext(o))) {
        Py_DECREF(context);
        if (context == value) {
          fail(0);
        }
        o = context;
        if (o == slow_o) {
          break;
        }
        if (slow_update_toggle) {
          slow_o = PyException_GetContext(slow_o);
          Py_DECREF(slow_o);
        }
        slow_update_toggle = !slow_update_toggle;
      }
      PyException_SetContext(value, exc_value);
    } else {
      Py_DECREF(exc_value);
    }
  }
  if (value != NULL && PyExceptionInstance_Check(value)) {
    tb = PyException_GetTraceback(value);
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
    Py_ssize_t i, n;
    n = PyTuple_Size(exc);
    for (i = 0; i < n; i++) {
      if (PyErr_GivenExceptionMatches(
          err, PyTuple_GET_ITEM(exc, i)))
      {
        return 1;
      }
    }
    return 0;
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

void _PyErr_Fetch(PyThreadState *tstate, PyObject **p_type, PyObject **p_value, PyObject **p_traceback) {
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
    exc = PyObject_Call(exception_type, value, NULL);
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

void _PyErr_SetKeyError(PyObject *arg) {
  PyThreadState *tstate = _PyThreadState_GET();
  PyObject *tup = PyTuple_Pack(1, arg);
  if (!tup) {
    return;
  }
  _PyErr_SetObject(tstate, PyExc_KeyError, tup);
  Py_DECREF(tup);
}

PyObject *
PyErr_SetFromErrnoWithFilenameObjects(PyObject *exc, PyObject *filenameObject, PyObject *filenameObject2) {
  PyThreadState *tstate = _PyThreadState_GET();
  PyObject *message;
  PyObject *v, *args;
  int i = errno;

  if (i != 0) {
    const char *s = strerror(i);
    // TODO call PyUnicode_DecodeLocale
    message = PyUnicode_FromString(s);
  } else {
    message = PyUnicode_FromString("Error");
  }
  if (message == NULL) {
    return NULL;
  }
  if (filenameObject != NULL) {
    if (filenameObject2 != NULL) {
      fail(0);
    } else {
      args = Py_BuildValue("(iOO)", i, message, filenameObject);
    }
  } else {
    fail(0);
  }
  Py_DECREF(message);

  if (args != NULL) {
    v = PyObject_Call(exc, args, NULL);
    Py_DECREF(args);
    if (v != NULL) {
      _PyErr_SetObject(tstate, (PyObject *) Py_TYPE(v), v);
      Py_DECREF(v);
    }
  }
  return NULL;
}


