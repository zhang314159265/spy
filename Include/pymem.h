#pragma once

#include <stdlib.h>

void *PyMem_Malloc(size_t size) {
	return malloc(size);
}

void *PyMem_Realloc(void *ptr, size_t new_size) {
	return realloc(ptr, new_size);
}

void *PyMem_Calloc(size_t nelem, size_t elsize) {
	return calloc(nelem, elsize);
}

void PyMem_Free(void *ptr) {
	free(ptr);
}

// TODO follow cpy
#define PyMem_RawFree PyMem_Free

#define PyMem_New(type, n) \
	((type*) PyMem_Malloc((n) * sizeof(type)))

#define PyMem_NEW(type, n) PyMem_New(type, n)

#include "cpython/pymem.h"

#include <wchar.h>

// defined in cpy/Objects/obmalloc.c
wchar_t *
_PyMem_RawWcsdup(const wchar_t *str) {
  assert(str != NULL);

  size_t len = wcslen(str);

  size_t size = (len + 1) * sizeof(wchar_t);
  wchar_t *str2 = PyMem_RawMalloc(size);
  if (str2 == NULL) {
    return NULL;
  }

  memcpy(str2, str, size);
  return str2;
}
