#pragma once

#include "longobject.h"

#define _PY_NSMALLPOSINTS 257
#define _PY_NSMALLNEGINTS 5

// The PyInterpreterState typedef is in Include/pystate.h
struct _is {
  PyLongObject *small_ints[_PY_NSMALLNEGINTS + _PY_NSMALLPOSINTS];
  struct pyruntimestate *runtime;
};
