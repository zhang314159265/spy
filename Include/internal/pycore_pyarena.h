#ifndef Py_INTERNAL_PYARENA_H
#define Py_INTERNAL_PYARENA_H

typedef struct _arena PyArena;

// defined in cpy/Python/pyarena.c
struct _arena {
};

void *
_PyArena_Malloc(PyArena *arena, size_t size) {
	return PyMem_Malloc(size);
}

#endif
