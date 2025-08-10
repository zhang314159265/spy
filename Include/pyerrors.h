#pragma once

#include "object.h"
#include "sutil.h"
#include <unistd.h>
#include <errno.h>

typedef struct _ts PyThreadState;

// Defined in cpy/Python/errors.c
PyObject *PyErr_NoMemory(void) {
	assert(false);
}

static inline PyObject *_PyErr_Occurred(PyThreadState *tstate);
static PyThreadState *_PyThreadState_GET(void);

PyObject *PyErr_Occurred(void) {
	// The caller must hold the GIL.
	// TODO add assert as cpy

	PyThreadState *tstate = _PyThreadState_GET();
	return _PyErr_Occurred(tstate);
}

extern PyObject *PyExc_AttributeError;
extern PyObject *PyExc_KeyError;
extern PyObject *PyExc_AssertionError;

PyObject *PyErr_Format(PyObject *exception, const char *format, ...);

#define PyExceptionClass_Check(x) \
  (PyType_Check((x)) && \
    PyType_FastSubclass((PyTypeObject *) (x), Py_TPFLAGS_BASE_EXC_SUBCLASS))

#define PyExceptionInstance_Check(x) \
  PyType_FastSubclass(Py_TYPE(x), Py_TPFLAGS_BASE_EXC_SUBCLASS)

#define PyExceptionInstance_Class(x) ((PyObject *) Py_TYPE(x))

#define PyException_HEAD PyObject_HEAD PyObject *dict; \
  PyObject *args; PyObject *traceback; \
  PyObject *context; PyObject *cause; \
  char suppress_context;

typedef struct {
  PyException_HEAD
} PyBaseExceptionObject;

typedef struct {
  PyException_HEAD
  PyObject *obj;
  PyObject *name;
} PyAttributeErrorObject;

typedef struct {
  PyException_HEAD
  PyObject *msg;
  PyObject *name;
  PyObject *path;
} PyImportErrorObject;

typedef struct {
  PyException_HEAD
  PyObject *myerrno;
  PyObject *strerror;
  PyObject *filename;
  PyObject *filename2;
  Py_ssize_t written;
} PyOSErrorObject;

void _PyErr_Fetch(PyThreadState *tstate, PyObject **p_type, PyObject **p_value, PyObject **p_traceback);
void _PyErr_NormalizeException(PyThreadState *tstate, PyObject **exc, PyObject **val, PyObject **tb);

static inline PyBaseExceptionObject *
_PyBaseExceptionObject_cast(PyObject *exc) {
  assert(PyExceptionInstance_Check(exc));
  return (PyBaseExceptionObject *) exc;
}

static int BaseException_set_tb(PyBaseExceptionObject *self, PyObject *tb, void *ignored);


int PyException_SetTraceback(PyObject *self, PyObject *tb) {
  return BaseException_set_tb(_PyBaseExceptionObject_cast(self), tb, NULL);
}

int PyErr_GivenExceptionMatches(PyObject *err, PyObject *exc);


void _PyErr_SetObject(PyThreadState *tstate, PyObject *exception, PyObject *value);

PyObject *
PyException_GetTraceback(PyObject *self) {
  PyBaseExceptionObject *base_self = _PyBaseExceptionObject_cast(self);
  Py_XINCREF(base_self->traceback);
  return base_self->traceback;
}

void _PyErr_SetKeyError(PyObject *arg);

void PyErr_Clear(void);

PyObject *PyException_GetContext(PyObject *self);

void PyException_SetContext(PyObject *self, PyObject *context);

PyObject *
PyErr_SetFromErrnoWithFilenameObjects(PyObject *exc, PyObject *filenameObject, PyObject *filenameObject2);

PyObject *
PyErr_SetFromErrnoWithFilenameObject(PyObject *exc, PyObject *filenameObject) {
  return PyErr_SetFromErrnoWithFilenameObjects(exc, filenameObject, NULL);
}

void _PyErr_Restore(PyThreadState *tstate, PyObject *type, PyObject *value, PyObject *traceback);

void
_PyErr_ChainExceptions(PyObject *exc, PyObject *val, PyObject *tb) {
  if (exc == NULL)
    return;

  PyThreadState *tstate = _PyThreadState_GET();
  
  if (!PyExceptionClass_Check(exc)) {
    fail(0);
  }

  if (_PyErr_Occurred(tstate)) {
    PyObject *exc2, *val2, *tb2;
    _PyErr_Fetch(tstate, &exc2, &val2, &tb2);
    _PyErr_NormalizeException(tstate, &exc, &val, &tb);
    if (tb != NULL) {
      fail(0);
    }
    Py_DECREF(exc);
    _PyErr_NormalizeException(tstate, &exc2, &val2, &tb2);
    PyException_SetContext(val2, val);
    _PyErr_Restore(tstate, exc2, val2, tb2);
  } else {
    _PyErr_Restore(tstate, exc, val, tb);
  }
}
