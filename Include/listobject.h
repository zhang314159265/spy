#pragma once

#include "cpython/listobject.h"

static PyObject *list_append(PyListObject *self, PyObject *object);
#include "Objects/clinic/listobject.c.h"

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

static int list_ass_item(PyListObject *a, Py_ssize_t i, PyObject *v);
static PyObject *list_repr(PyListObject *v);

static PySequenceMethods list_as_sequence = {
	.sq_ass_item = (ssizeobjargproc) list_ass_item,
};

extern PyTypeObject PyList_Type;

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
		// printf("allocated %ld, newsize %ld\n", allocated, newsize);
		assert(self->ob_item != NULL || newsize == 0);
		Py_SET_SIZE(self, newsize);
		return 0;
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
	PyObject *recycle_on_stack[8];
	PyObject **recycle = recycle_on_stack;
	PyObject **item = NULL;
  PyObject *v_as_SF = NULL; // PySequence_Fast(v)
  Py_ssize_t n; /* # of elements in replacement list */
  Py_ssize_t norig;
  Py_ssize_t d; // change in size
	Py_ssize_t k;
	size_t s;
	int result = -1;
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
	item = a->ob_item;
	s = norig * sizeof(PyObject *);
	if (s) {
		if (s > sizeof(recycle_on_stack)) {
			assert(false);
		}
		memcpy(recycle, &item[ilow], s);
	}

	if (d < 0) { /* Delete -d items */
		Py_ssize_t tail;
		tail = (Py_SIZE(a) - ihigh) * sizeof(PyObject *);
		memmove(&item[ihigh + d], &item[ihigh], tail);
		if (list_resize(a, Py_SIZE(a) + d) < 0) {
			assert(false);
		}
		item = a->ob_item;
	} else if (d > 0) { /* Insert d items */
		assert(false);
	}
	for (k = 0; k < n; k++, ilow++) {
		assert(false);
	}
	for (k = norig -1; k >= 0; --k)
		Py_XDECREF(recycle[k]);
	result = 0;
Error:
	if (recycle != recycle_on_stack)
		PyMem_Free(recycle);
	Py_XDECREF(v_as_SF);
	return result;
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

static int list_ass_item(PyListObject *a, Py_ssize_t i, PyObject *v) {
	if (v == NULL)
		return list_ass_slice(a, i, i + 1, v);
	assert(false);
}

static PyObject *list_repr(PyListObject *v) {
	Py_ssize_t i;
	PyObject *s;
	_PyUnicodeWriter writer;

	if (Py_SIZE(v) == 0) {
		return PyUnicode_FromString("[]");
	}

	_PyUnicodeWriter_Init(&writer);
	writer.overallocate = 1;
	writer.min_length = 1 + 1 + (2 + 1) * (Py_SIZE(v) - 1) + 1;

	if (_PyUnicodeWriter_WriteChar(&writer, '[') < 0)
		assert(false);
	for (i = 0; i < Py_SIZE(v); ++i) {
		if (i > 0) {
			if (_PyUnicodeWriter_WriteASCIIString(&writer, ", ", 2) < 0)
				assert(false);
		}

		s = PyObject_Repr(v->ob_item[i]);
		if (s == NULL)
			assert(false);

		if (_PyUnicodeWriter_WriteStr(&writer, s) < 0) {
			assert(false);
		}
		Py_DECREF(s);
	}

	writer.overallocate = 0;
	if (_PyUnicodeWriter_WriteChar(&writer, ']') < 0)
		assert(false);
	
	return _PyUnicodeWriter_Finish(&writer);
}

static PyObject *list_append(PyListObject *self, PyObject *object) {
	if (app1(self, object) == 0)
		Py_RETURN_NONE;
	return NULL;
}

static int
list_preallocate_exact(PyListObject *self, Py_ssize_t size) {
	assert(self->ob_item == NULL);
	assert(size > 0);

	size = (size + 1) & ~(size_t)1;
	PyObject **items = PyMem_New(PyObject *, size);
	if (items == NULL) {
		assert(false);
	}
	self->ob_item = items;
	self->allocated = size;
	return 0;
}

PyObject *PySequence_Fast(PyObject *o, const char *m);

// copied from abstract.h
#define PySequence_Fast_GET_SIZE(o) \
	(PyList_Check(o) ? PyList_GET_SIZE(o) : PyTuple_GET_SIZE(o))

#define PySequence_Fast_ITEMS(sf) \
	(PyList_Check(sf) ? ((PyListObject *)(sf))->ob_item \
		: ((PyTupleObject *)(sf))->ob_item)

PyObject *PyObject_GetIter(PyObject * o);
Py_ssize_t PyObject_LengthHint(PyObject *o, Py_ssize_t defaultvalue);

static PyObject *
list_extend(PyListObject *self, PyObject *iterable) {
	PyObject *it;
	Py_ssize_t m;  // size of self
	Py_ssize_t n; // guess for size of iterable
	Py_ssize_t i;
	PyObject *(*iternext)(PyObject *);

	if (PyList_CheckExact(iterable) || PyTuple_CheckExact(iterable) ||
				(PyObject *) self == iterable) {
		PyObject **src, **dest;

		iterable = PySequence_Fast(iterable, "argument must be iterable");
		if (!iterable)
			return NULL;
		n = PySequence_Fast_GET_SIZE(iterable);
		if (n == 0) {
			assert(false);
		}
		m = Py_SIZE(self);
		if (self->ob_item == NULL) {
			if (list_preallocate_exact(self, n) < 0) {
				return NULL;
			}
			Py_SET_SIZE(self, n);
		} else if (list_resize(self, m + n) < 0) {
			assert(false);
		}
		src = PySequence_Fast_ITEMS(iterable);
		dest = self->ob_item + m;
		for (i = 0; i < n; i++) {
			PyObject *o = src[i];
			Py_INCREF(o);
			dest[i] = o;
		}
		Py_DECREF(iterable);
		Py_RETURN_NONE;
	}

	it = PyObject_GetIter(iterable);
	if (it == NULL)
		return NULL;
	iternext = *Py_TYPE(it)->tp_iternext;

	// Guess a result list size
	n = PyObject_LengthHint(iterable, 8);
	if (n < 0) {
		Py_DECREF(it);
		return NULL;
	}
	m = Py_SIZE(self);

	if (self->ob_item == NULL) {
		if (n && list_preallocate_exact(self, n) < 0)
			assert(false);
	} else {
		// make room
		assert(false);
	}

	// Run iterator to exhausion
	for (;;) {
		PyObject *item = iternext(it);
		if (item == NULL) {
			if (PyErr_Occurred()) {
				assert(false);
			}
			break;
		}
		if (Py_SIZE(self) < self->allocated) {
			PyList_SET_ITEM(self, Py_SIZE(self), item);
			Py_SET_SIZE(self, Py_SIZE(self) + 1);
		} else {
			assert(false);
		}
	}
	if (Py_SIZE(self) < self->allocated) {
		if (list_resize(self, Py_SIZE(self)) < 0)
			assert(false);
	}

	Py_DECREF(it);
	Py_RETURN_NONE;
}

PyObject *
_PyList_Extend(PyListObject *self, PyObject *iterable) {
	return list_extend(self, iterable);
}
