#include "pymacro.h"

#define _PyObject_SIZE(typeobj) ((typeobj)->tp_basicsize)

#define _PyObject_VAR_SIZE(typeobj, nitems) \
  _Py_SIZE_ROUND_UP((typeobj)->tp_basicsize + \
    (nitems) * (typeobj)->tp_itemsize, \
    SIZEOF_VOID_P)
