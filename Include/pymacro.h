#ifndef Py_PYMACRO_H
#define Py_PYMACRO_H

#define Py_MIN(x, y) (((x) > (y)) ? (y) : (x))
#define Py_MAX(x, y) (((x) > (y)) ? (x) : (y))

#define Py_CHARMASK(c) ((unsigned char) ((c) & 0xFF))

#define Py_UNREACHABLE() assert(0)

// Check if pointer 'p' is aligned to 'a'-bytes boundary
#define _Py_IS_ALIGNED(p, a) (!((uintptr_t)(p) & (uintptr_t)((a) - 1)))

#endif
