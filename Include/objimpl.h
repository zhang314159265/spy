#ifndef Py_OBJIMPL_H
#define Py_OBJIMPL_H

#include <stdlib.h>

// defined in cpy/Objects/obmalloc.c
void *
PyObject_Malloc(size_t size) {
	return malloc(size);
}

void PyObject_Free(void *ptr) {
  free(ptr); // TODO not consistent with cpy
}

void *PyObject_Calloc(size_t nelem, size_t elsize) {
  return calloc(nelem, elsize);
}

void *
PyObject_Realloc(void *ptr, size_t new_size)
{
  return realloc(ptr, new_size);
}

#define PyObject_New(type, typeobj) ((type *) _PyObject_New(typeobj))
#define PyObject_Del PyObject_Free

#include "cpython/objimpl.h"
#include "internal/pycore_object.h"

// implemented in cpy/Objects/object.c
PyObject *
_PyObject_New(PyTypeObject *tp) {
  PyObject *op = (PyObject *) PyObject_Malloc(_PyObject_SIZE(tp));
  if (op == NULL) {
    assert(false);
  }
  _PyObject_Init(op, tp);
  return op;
}

static PyObject *
_PyObject_GC_Alloc(int use_calloc, size_t basicsize) {
  // TODO: allocate the GC metadata as cpytion
  PyObject *op;
  size_t size = basicsize;
  if (use_calloc) {
    // op = PyObject_Calloc(1, size);
    assert(false);
  } else {
    op = PyObject_Malloc(size);
  }
  assert(op);
  return op;
}

PyObject *
_PyObject_GC_Malloc(size_t basicsize) {
  return _PyObject_GC_Alloc(0, basicsize);
}

// defined in cpy/Modules/gcmodule.c
PyObject *_PyObject_GC_New(PyTypeObject *tp) {
  PyObject *op = _PyObject_GC_Malloc(_PyObject_SIZE(tp));
  if (op == NULL) {
    return NULL;
  }
  _PyObject_Init(op, tp);
  return op;
}

PyVarObject *_PyObject_GC_NewVar(PyTypeObject *tp, Py_ssize_t nitems) {
  size_t size;
  PyVarObject *op;

  if (nitems < 0) {
    assert(false);
  }
  size = _PyObject_VAR_SIZE(tp, nitems);
  op = (PyVarObject *) _PyObject_GC_Malloc(size);
  if (op == NULL) {
    return NULL;
  }
  _PyObject_InitVar(op, tp, nitems);
  return op;
}

#define PyObject_GC_New(type, typeobj) \
  ((type *) _PyObject_GC_New(typeobj))

#define PyObject_GC_NewVar(type, typeobj, n) \
    ((type *) _PyObject_GC_NewVar((typeobj), (n)))

// defined in cpy/Modules/gcmodule.c
void PyObject_GC_Del(void *op) {
  // TODO handle GC related stuff
  PyObject_Free(op);
}

#endif
