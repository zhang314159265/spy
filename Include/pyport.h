#ifndef Py_PYPORT_H
#define Py_PYPORT_H

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

#endif
