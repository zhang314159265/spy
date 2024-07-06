#ifndef Py_PYMEM_H
#define Py_PYMEM_H

#include <stdlib.h>

void *PyMem_Malloc(size_t size) {
	return malloc(size);
}

void *PyMem_Realloc(void *ptr, size_t new_size) {
	return realloc(ptr, new_size);
}

#endif
