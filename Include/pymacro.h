#ifndef Py_PYMACRO_H
#define Py_PYMACRO_H

#define Py_MIN(x, y) (((x) > (y)) ? (y) : (x))
#define Py_MAX(x, y) (((x) > (y)) ? (x) : (y))

#define Py_ABS(x) ((x) < 0 ? -(x) : (x))

#define Py_CHARMASK(c) ((unsigned char) ((c) & 0xFF))

#define Py_UNREACHABLE() assert(0)

// Check if pointer 'p' is aligned to 'a'-bytes boundary
#define _Py_IS_ALIGNED(p, a) (!((uintptr_t)(p) & (uintptr_t)((a) - 1)))

// a should be a power of 2
#define _Py_SIZE_ROUND_UP(n, a) (((size_t)(n) + \
		(size_t)((a) - 1)) & ~(size_t)((a) - 1))

#define PyDoc_STRVAR(name, str) PyDoc_VAR(name) = PyDoc_STR(str)

#define PyDoc_VAR(name) static const char name[]
#define PyDoc_STR(str) str

#define Py_ARRAY_LENGTH(array) \
  (sizeof(array) / sizeof((array)[0]))

#define _Py_SIZE_ROUND_DOWN(n, a) ((size_t)(n) & ~(size_t)((a) - 1))

#endif
