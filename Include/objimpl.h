#ifndef Py_OBJIMPL_H
#define Py_OBJIMPL_H

// defined in cpy/Objects/obmalloc.c
void *
PyObject_Malloc(size_t size) {
	return malloc(size);
}

#endif
