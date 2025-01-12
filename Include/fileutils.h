#pragma once

#include <sys/stat.h>

#define _Py_stat_struct stat

int
_Py_fstat_noraise(int fd, struct _Py_stat_struct *status) {
  return fstat(fd, status);
}

Py_ssize_t 
_Py_read(int fd, void *buf, size_t count) {
  Py_ssize_t n;
  int err;

  assert(!PyErr_Occurred());

  #if 0
  if (count > _PY_READ_MAX) {
    count = _PY_READ_MAX;
  }
  #endif

  do {
    errno = 0;
    n = read(fd, buf, count);
    err = errno;
  } while (n < 0 && err == EINTR);

  if (n < 0) {
    fail(0);
  }
  return n;
}
