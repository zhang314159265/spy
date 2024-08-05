#pragma once

typedef struct {
	void *ctx;
	void *(*malloc)(void *ctx, size_t size);
	void *(*calloc)(void *ctx, size_t nelem, size_t elsize);
} PyMemAllocatorEx;

static void *
_PyMem_RawCalloc(void *ctx, size_t nelem, size_t elsize) {
	return calloc(nelem, elsize);
}

static void *
_PyMem_RawMalloc(void *ctx, size_t size) {
	if (size == 0) {
		size = 1;
	}
	return malloc(size);
}

#define MALLOC_ALLOC { \
	.ctx = NULL, \
	.malloc = _PyMem_RawMalloc, \
	.calloc = _PyMem_RawCalloc, \
}

#define PYRAW_ALLOC MALLOC_ALLOC

static PyMemAllocatorEx _PyMem_Raw = PYRAW_ALLOC;

// defined in cpy/Objects/obmalloc.c
void *PyMem_RawCalloc(size_t nelem, size_t elsize) {
	return _PyMem_Raw.calloc(_PyMem_Raw.ctx, nelem, elsize);
}

void *PyMem_RawMalloc(size_t size) {
	return _PyMem_Raw.malloc(_PyMem_Raw.ctx, size);
}
