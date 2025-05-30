#pragma once

typedef ssize_t Py_ssize_t;
typedef Py_ssize_t Py_hash_t;
typedef size_t Py_uhash_t;

#ifndef PYLONG_BITS_IN_DIGIT
#if SIZEOF_VOID_P >= 8
#define PYLONG_BITS_IN_DIGIT 30
#else
#define PYLONG_BITS_IN_DIGIT 15
#endif
#endif

#define Py_SAFE_DOWNCAST(VALUE, WIDE, NARROW) (NARROW)(VALUE)

#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

#ifndef INT_MIN
#define INT_MIN -2147483648
#endif

#define SIZEOF_LONG 8

#ifndef LONG_MAX
#if SIZEOF_LONG == 4
#define LONG_MAX 0x7FFFFFFFL
#elif SIZEOF_LONG == 8
#define LONG_MAX 0x7FFFFFFFFFFFFFFFL
#else
#error "could not set LONG_MAX in pyport.h"
#endif
#endif

#define PY_SSIZE_T_MAX ((Py_ssize_t)(((size_t) -1) >> 1))
#define PY_SSIZE_T_MIN (-PY_SSIZE_T_MAX - 1)

// TODO follow cpy
#define PyMODINIT_FUNC PyObject *
