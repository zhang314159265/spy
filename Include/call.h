#pragma once

static PyObject * _PyObject_CallFunctionVa(PyThreadState *tstate, PyObject *callable,
    const char *format, va_list va, int is_size_t);

static PyObject *
callmethod(PyThreadState *tstate, PyObject *callable, const char *format, va_list va, int is_size_t) {
  assert(callable != NULL);
  if (!PyCallable_Check(callable)) {
    fail(0);
  }

  return _PyObject_CallFunctionVa(tstate, callable, format, va, is_size_t);
}

PyObject *PyObject_CallMethod(PyObject *obj, const char *name, const char *format, ...) {
  PyThreadState *tstate = _PyThreadState_GET();

  if (obj == NULL || name == NULL) {
    fail(0);
  }

  PyObject *callable = PyObject_GetAttrString(obj, name);
  if (callable == NULL) {
    return NULL;
  }

  // printf("PyObject_CallMethod get callable of type %s\n", Py_TYPE(callable)->tp_name);
  va_list va;
  va_start(va, format);
  PyObject *retval = callmethod(tstate, callable, format, va, 0);
  va_end(va);

  Py_DECREF(callable);
  return retval;
}

static inline PyObject *
_PyObject_CallNoArgTstate(PyThreadState *tstate, PyObject *func) {
  return _PyObject_VectorcallTstate(tstate, func, NULL, 0, NULL);
}

PyObject * _PyObject_CallMethodId(PyObject *obj, _Py_Identifier *name,
    const char *format, ...);

PyObject *PyObject_CallFunction(PyObject *callable, const char *format, ...);
