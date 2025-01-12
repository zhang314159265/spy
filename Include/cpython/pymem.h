#pragma once

typedef struct {
	void *ctx;
	void *(*malloc)(void *ctx, size_t size);
	void *(*calloc)(void *ctx, size_t nelem, size_t elsize);
  void *(*realloc)(void *ctx, void *ptr, size_t new_size);
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

static void *
_PyMem_RawRealloc(void *ctx, void *ptr, size_t size) {
  if (size == 0)
    size = 1;
  return realloc(ptr, size);
}

#define MALLOC_ALLOC { \
	.ctx = NULL, \
	.malloc = _PyMem_RawMalloc, \
	.calloc = _PyMem_RawCalloc, \
  .realloc = _PyMem_RawRealloc, \
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

void *PyMem_RawRealloc(void *ptr, size_t new_size) {
  if (new_size > (size_t) PY_SSIZE_T_MAX) {
    return NULL;
  }
  return _PyMem_Raw.realloc(_PyMem_Raw.ctx, ptr, new_size);
}
