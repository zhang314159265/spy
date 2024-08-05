#pragma once

#include "cpython/listobject.h"

#define PyList_Check(op) \
  PyType_FastSubclass(Py_TYPE(op), Py_TPFLAGS_LIST_SUBCLASS)

#define _PyList_CAST(op) (assert(PyList_Check(op)), (PyListObject *) (op))

#define PyList_GET_SIZE(op) Py_SIZE(_PyList_CAST(op))

#define PyList_SET_ITEM(op, i, v) ((void) (_PyList_CAST(op)->ob_item[i] = (v)))
#define PyList_GET_ITEM(op, i) (_PyList_CAST(op)->ob_item[i])

static void
list_dealloc(PyListObject *op) {
	Py_ssize_t i;
	// PyObject_GC_UnTrack(op);

	if (op->ob_item != NULL) {
		i = Py_SIZE(op);
		while (--i >= 0) {
			Py_XDECREF(op->ob_item[i]);
		}
		PyMem_Free(op->ob_item);
	}
	Py_TYPE(op)->tp_free((PyObject *) op);
}

// defined in cpy/Objects/listobject.c
PyTypeObject PyList_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "list",
  .tp_basicsize = sizeof(PyListObject),
  .tp_flags = Py_TPFLAGS_LIST_SUBCLASS,
	.tp_dealloc = (destructor) list_dealloc,
	.tp_free = PyObject_GC_Del,
};

// defined in cpy/Objects/listobject.c
PyObject *PyList_New(Py_ssize_t size) {
  assert(size >= 0);

  PyListObject *op = PyObject_GC_New(PyListObject, &PyList_Type);
  if (op == NULL) {
    return NULL;
  }
  if (size <= 0) {
    op->ob_item = NULL;
  } else {
		op->ob_item = (PyObject **) PyMem_Calloc(size, sizeof(PyObject *));
		if (op->ob_item == NULL) {
			assert(false);
		}
  }
  Py_SET_SIZE(op, size);
  op->allocated = size;
  _PyObject_GC_TRACK(op);
  return (PyObject *) op;
}

static int
list_resize(PyListObject *self, Py_ssize_t newsize) {
  PyObject **items;
  size_t new_allocated, num_allocated_bytes;
  Py_ssize_t allocated = self->allocated;

  if (allocated >= newsize && newsize >= (allocated >> 1)) {
    assert(false);
  }

  new_allocated = ((size_t) newsize + (newsize >> 3) + 6) & ~(size_t) 3;

  if (newsize == 0) {
    new_allocated = 0;
  }
  num_allocated_bytes = new_allocated * sizeof(PyObject *);
  items = (PyObject **) PyMem_Realloc(self->ob_item, num_allocated_bytes);
  assert(items);
  self->ob_item = items;
  Py_SET_SIZE(self, newsize);
  self->allocated = new_allocated;
  return 0;
}

static int
app1(PyListObject *self, PyObject *v) {
  Py_ssize_t n = PyList_GET_SIZE(self);

  assert(v != NULL);
  if (list_resize(self, n + 1) < 0) {
    return -1;
  }
  Py_INCREF(v);
  PyList_SET_ITEM(self, n, v);
  return 0;
}

int PyList_Append(PyObject *op, PyObject *newitem) {
  if (PyList_Check(op) && (newitem != NULL)) {
    return app1((PyListObject *) op, newitem);
  }
  assert(false);
}

static int
_list_clear(PyListObject *a) {
  Py_ssize_t i;
  PyObject **item = a->ob_item;
  if (item != NULL) {
    i = Py_SIZE(a);
    Py_SET_SIZE(a, 0);
    a->ob_item = NULL;
    a->allocated = 0;
    while (--i >= 0) {
      Py_XDECREF(item[i]);
    }
    PyMem_Free(item);
  }
  return 0;
}

static int
list_ass_slice(PyListObject *a, Py_ssize_t ilow, Py_ssize_t ihigh, PyObject *v) {
  PyObject *v_as_SF = NULL; // PySequence_Fast(v)
  Py_ssize_t n; /* # of elements in replacement list */
  Py_ssize_t norig;
  Py_ssize_t d; // change in size
#define b ((PyListObject *) v)
  if (v == NULL) {
    n = 0;
  } else {
    assert(false);
  }
  if (ilow < 0)
    ilow = 0;
  else if (ilow > Py_SIZE(a))
    ilow = Py_SIZE(a);

  if (ihigh < ilow)
    ihigh = ilow;
  else if (ihigh > Py_SIZE(a))
    ihigh = Py_SIZE(a);

  norig = ihigh - ilow;
  assert(norig >= 0);
  d = n - norig;
  if (Py_SIZE(a) + d == 0) {
    Py_XDECREF(v_as_SF);
    return _list_clear(a);
  }
  assert(false);
#undef b
}

int PyList_SetSlice(PyObject *a, Py_ssize_t ilow, Py_ssize_t ihigh, PyObject *v) {
  if (!PyList_Check(a)) {
    assert(false);
  }
  return list_ass_slice((PyListObject *) a, ilow, ihigh, v);
}

#define PyList_CheckExact(op) Py_IS_TYPE(op, &PyList_Type)

PyObject * _PyTuple_FromArray(PyObject *const *src, Py_ssize_t n);

PyObject *PyList_AsTuple(PyObject *v) {
	if (v == NULL || !PyList_Check(v)) {
		assert(false);
	}
	return _PyTuple_FromArray(((PyListObject *)v)->ob_item, Py_SIZE(v));
}

Py_ssize_t
PyList_Size(PyObject *op) {
	if (!PyList_Check(op)) {
		assert(false);
	} else
		return Py_SIZE(op);
}
