#pragma once

typedef PyObject *(*allocfunc)(PyTypeObject *, Py_ssize_t);

typedef struct {
  binaryfunc nb_or;
  binaryfunc nb_inplace_or;
} PyNumberMethods;

struct _typeobject {
	PyObject_VAR_HEAD
	const char *tp_name; // for printing, in format "<module>.<name>"
  Py_ssize_t tp_basicsize, tp_itemsize;
	unsigned long tp_flags;

  destructor tp_dealloc;

  allocfunc tp_alloc;

  PyObject *tp_mro; // method resolution order
  PyNumberMethods *tp_as_number;

  struct _typeobject *tp_base;

  getiterfunc tp_iter;
  iternextfunc tp_iternext;
  freefunc tp_free; // Low-level free-memory routine

  PyObject *tp_dict;
  PyObject *tp_bases;

  hashfunc tp_hash;

  richcmpfunc tp_richcompare;
};

// The *real* layout of a type object when allocated on the heap
typedef struct _heaptypeobject {
  PyTypeObject ht_type;
} PyHeapTypeObject;

// defined in cpy/Objects/object.c
PyObject *_PyObject_NextNotImplemented(PyObject *self) {
  assert(false);
}
