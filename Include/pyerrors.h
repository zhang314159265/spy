#ifndef Py_ERRORS_H
#define Py_ERRORS_H

#include "object.h"
#include "sutil.h"

// Defined in cpy/Python/errors.c
PyObject *PyErr_NoMemory(void) {
	assert(false);
}

PyObject *PyErr_Occurred(void) {
	// assert(false);
  printf("WARNING: PyErr_Occurred not implemented yet\n");
  return NULL;
}

#endif
