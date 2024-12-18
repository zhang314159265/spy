#ifndef Py_ERRORS_H
#define Py_ERRORS_H

#include "object.h"
#include "sutil.h"

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

#endif
