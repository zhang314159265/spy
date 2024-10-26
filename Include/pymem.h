#ifndef Py_PYMEM_H
#define Py_PYMEM_H

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

#define PyMem_New(type, n) \
	((type*) PyMem_Malloc((n) * sizeof(type)))

#define PyMem_NEW(type, n) PyMem_New(type, n)

#endif
