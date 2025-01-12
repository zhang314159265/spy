#pragma once

typedef int64_t _PyTime_t;

PyObject *
_PyLong_FromTime_t(time_t t) {
#if SIZEOF_TIME_T == SIZEOF_LONG_LONG
  return PyLong_FromLongLong((long long) t);
#else
  fail(0);
#endif
}
