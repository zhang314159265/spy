#pragma once

#include "pystate.h"
#include "internal/pycore_interp.h"
#include "internal/pycore_pystate.h"

static inline PyObject* __PyLong_GetSmallInt_internal(int value) {
  PyInterpreterState *interp = _PyInterpreterState_GET();
  assert(-_PY_NSMALLNEGINTS <= value && value < _PY_NSMALLPOSINTS);
  size_t index = _PY_NSMALLNEGINTS + value;
  PyObject *obj = (PyObject*) interp->small_ints[index];
  assert(obj != NULL);
  return obj;
}
