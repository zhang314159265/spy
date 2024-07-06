#ifndef Py_PYMACRO_H
#define Py_PYMACRO_H

#define Py_MIN(x, y) (((x) > (y)) ? (y) : (x))
#define Py_MAX(x, y) (((x) > (y)) ? (x) : (y))

#define Py_CHARMASK(c) ((unsigned char) ((c) & 0xFF))

#define Py_UNREACHABLE() assert(0)

#endif
