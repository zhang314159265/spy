#pragma once

#include <stdarg.h>

// defined in cpy/Include/cpython/tupleobject.h
typedef struct {
	PyObject_VAR_HEAD
	PyObject *ob_item[1];
} PyTupleObject;

typedef struct {
  PyObject_HEAD
  Py_ssize_t it_index;
  PyTupleObject *it_seq;
} tupleiterobject;

static PyTupleObject *tuple_alloc(Py_ssize_t size);
static inline void tuple_gc_track(PyTupleObject *op);

PyObject *PyTuple_Pack(Py_ssize_t n, ...) {
	Py_ssize_t i;
	PyObject *o;
	PyObject **items;
	va_list vargs;

	if (n == 0) {
		assert(false);
	}

	va_start(vargs, n);
	PyTupleObject *result = tuple_alloc(n);
	if (result == NULL) {
		assert(false);
	}
	items = result->ob_item;
	for (i = 0; i < n; i++) {
		o = va_arg(vargs, PyObject *);
		Py_INCREF(o);
		items[i] = o;
	}
	va_end(vargs);
	tuple_gc_track(result);
	return (PyObject *) result;
}

static Py_hash_t tuplehash(PyTupleObject *v);
static PyObject *tuplerichcompare(PyObject *v, PyObject *w, int op);
static void tupledealloc(PyTupleObject *op);
static Py_ssize_t tuplelength(PyTupleObject *a) { return Py_SIZE(a); }
static PyObject *tupleitem(PyTupleObject *a, Py_ssize_t i);
static PyObject *tuplerepr(PyTupleObject *v);

static PySequenceMethods tuple_as_sequence = {
	.sq_length = (lenfunc) tuplelength,
	.sq_item = (ssizeargfunc) tupleitem,
};

static PyObject *tuple_new(PyTypeObject *type, PyObject *args, PyObject *kwargs);
static PyObject *tuple_iter(PyObject *seq);

PyTypeObject PyTuple_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "tuple",
	.tp_basicsize = sizeof(PyTupleObject) - sizeof(PyObject *),
	.tp_itemsize = sizeof(PyObject *),
	.tp_flags = Py_TPFLAGS_TUPLE_SUBCLASS,
	.tp_hash = (hashfunc) tuplehash,
	.tp_richcompare = tuplerichcompare,
	.tp_dealloc = (destructor) tupledealloc,
	.tp_free = PyObject_GC_Del,
	.tp_as_sequence = &tuple_as_sequence,
	.tp_repr = (reprfunc) tuplerepr,
  .tp_new = tuple_new,
  .tp_iter = tuple_iter,
};

static PyTupleObject *
tuple_alloc(Py_ssize_t size) {
	PyTupleObject *op;
	if (size < 0) {
		assert(false);
	}

	op = PyObject_GC_NewVar(PyTupleObject, &PyTuple_Type, size);
	if (op == NULL) {
		return NULL;
	}
	return op;
}

static inline void
tuple_gc_track(PyTupleObject *op) {
	_PyObject_GC_TRACK(op);
}

PyObject *PyTuple_New(Py_ssize_t size) {
	PyTupleObject *op;
	op = tuple_alloc(size);
	if (op == NULL) {
		return NULL;
	}
	for (Py_ssize_t i = 0; i < size; i++) {
		op->ob_item[i] = NULL;
	}
	tuple_gc_track(op);
	return (PyObject *) op;
}

#define PyTuple_Check(op) \
	PyType_FastSubclass(Py_TYPE(op), Py_TPFLAGS_TUPLE_SUBCLASS)

// defined in cpy/Include/cpython/tupleobject.h
#define _PyTuple_CAST(op) (assert(PyTuple_Check(op)), (PyTupleObject *)(op))
#define PyTuple_GET_SIZE(op) Py_SIZE(_PyTuple_CAST(op))

#define PyTuple_GET_ITEM(op, i) (_PyTuple_CAST(op)->ob_item[i])
#define PyTuple_SET_ITEM(op, i, v) ((void)(_PyTuple_CAST(op)->ob_item[i] = v))

#define PyTuple_CheckExact(op) Py_IS_TYPE(op, &PyTuple_Type)

static PyObject *
tuple_get_empty(void) {
  return PyTuple_New(0);
}

PyObject *
_PyTuple_FromArray(PyObject *const *src, Py_ssize_t n) {
	if (n == 0) {
    return tuple_get_empty();
	}

	#if 0
	printf("n %d\n", n);
	for (int i = 0; i < n; ++i) {
		PyTypeObject *typeobj = (PyTypeObject *) src[i];
		printf("%s\n", typeobj->tp_name);
	}
	#endif

	PyTupleObject *tuple = tuple_alloc(n);
	if (tuple == NULL) {
		return NULL;
	}
	PyObject **dst = tuple->ob_item;
	for (Py_ssize_t i = 0; i < n; i++) {
		PyObject *item = src[i];
		Py_INCREF(item);
		dst[i] = item;
	}
	tuple_gc_track(tuple);
	return (PyObject *) tuple;
}

static Py_hash_t tuplehash(PyTupleObject *v) {
	Py_ssize_t i, len = Py_SIZE(v);
	PyObject **item = v->ob_item;

	// a poor version

	Py_uhash_t acc = 1000000007;
	for (i = 0; i < len; i++) {
		Py_uhash_t lane = PyObject_Hash(item[i]);
		acc = acc ^ lane;
	}
	return acc;
}

static void tupledealloc(PyTupleObject *op) {
	Py_ssize_t len = Py_SIZE(op);
	// PyObject_GC_UnTrack(op);
	if (len > 0) {
		Py_ssize_t i = len;
		while (--i >= 0) {
			Py_XDECREF(op->ob_item[i]);
		}
	}
	Py_TYPE(op)->tp_free((PyObject *) op);
}

PyObject *
PyTuple_GetItem(PyObject *op, Py_ssize_t i) {
	if (!PyTuple_Check(op)) {
		assert(false);
	}
	if (i < 0 || i >= Py_SIZE(op)) {
		assert(false);
	}
	return ((PyTupleObject *) op)->ob_item[i];
}

Py_ssize_t
PyTuple_Size(PyObject *op) {
	if (!PyTuple_Check(op)) {
		assert(false);
	} else
		return Py_SIZE(op);
}

static PyObject *tupleitem(PyTupleObject *a, Py_ssize_t i) {
	if (i < 0 || i >= Py_SIZE(a)) {
		assert(false);
	}
	Py_INCREF(a->ob_item[i]);
	return a->ob_item[i];
}

static PyObject *tuplerepr(PyTupleObject *v) {
	Py_ssize_t i, n;
	_PyUnicodeWriter writer;

	n = Py_SIZE(v);
	if (n == 0) {
		return PyUnicode_FromString("()");
	}

	_PyUnicodeWriter_Init(&writer);
	writer.overallocate = 1;
	if (Py_SIZE(v) > 1) {
		writer.min_length = 1 + 1 + (2 + 1) * (Py_SIZE(v) - 1) + 1;
	} else {
		/* "(1,)" */
		writer.min_length = 4;
	}

	if (_PyUnicodeWriter_WriteChar(&writer, '(') < 0)
		assert(false);
	
	for (i = 0; i < n; ++i) {
		PyObject *s;

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
	if (n > 1) {
		if (_PyUnicodeWriter_WriteChar(&writer, ')') < 0)
			assert(false);
	} else {
		assert(false);
	}
	return _PyUnicodeWriter_Finish(&writer);
}

PyObject *PyTuple_GetSlice(PyObject *op, Py_ssize_t i, Py_ssize_t j);

int _PyTuple_Resize(PyObject **pv, Py_ssize_t newsize);

static PyObject *tupleiter_next(tupleiterobject *it);
static void tupleiter_dealloc(tupleiterobject *it);

PyTypeObject PyTupleIter_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "tuple_iterator",
  .tp_basicsize = sizeof(tupleiterobject),
  .tp_dealloc = (destructor) tupleiter_dealloc,
  .tp_getattro = PyObject_GenericGetAttr,
  .tp_iter = PyObject_SelfIter,
  .tp_iternext = (iternextfunc) tupleiter_next,
};

static PyObject *tuple_iter(PyObject *seq) {
  tupleiterobject *it;

  if (!PyTuple_Check(seq)) {
    assert(false);
  }
  it = PyObject_GC_New(tupleiterobject, &PyTupleIter_Type);
  if (it == NULL)
    return NULL;
  it->it_index = 0;
  Py_INCREF(seq);
  it->it_seq = (PyTupleObject *) seq;
  return (PyObject *) it;
}

static PyObject *tupleiter_next(tupleiterobject *it) {
  PyTupleObject *seq;
  PyObject *item;

  assert(it != NULL);
  seq = it->it_seq;
  if (seq == NULL)
    return NULL;
  assert(PyTuple_Check(seq));

  if (it->it_index < PyTuple_GET_SIZE(seq)) {
    item = PyTuple_GET_ITEM(seq, it->it_index);
    ++it->it_index;
    Py_INCREF(item);
    return item;
  }

  it->it_seq = NULL;
  Py_DECREF(seq);
  return NULL;
}

static void tupleiter_dealloc(tupleiterobject *it) {
  Py_XDECREF(it->it_seq);
  PyObject_GC_Del(it);
}
