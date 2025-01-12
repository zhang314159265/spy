#pragma once

#define LIST_EXTEND_METHODDEF \
  {"extend", (PyCFunction)list_extend, METH_O, ""},

static PyObject *list_extend(PyListObject *self, PyObject *iterable);

static PyMethodDef list_methods[] = {
	LIST_APPEND_METHODDEF
  LIST_EXTEND_METHODDEF
	{NULL, NULL},
};

static PyObject *list_subscript(PyListObject *self, PyObject *item);
static int list_ass_subscript(PyListObject *self, PyObject *item, PyObject *value);

static PyMappingMethods list_as_mapping = {
	.mp_subscript = (binaryfunc) list_subscript,
	.mp_ass_subscript = (objobjargproc) list_ass_subscript,
};

typedef struct {
  PyObject_HEAD
  Py_ssize_t it_index;
  PyListObject *it_seq;
} listiterobject;

static PyMethodDef listiter_methods[] = {
  {NULL, NULL}
};

static void
listiter_dealloc(listiterobject *it) {
  Py_XDECREF(it->it_seq);
  PyObject_GC_Del(it);
}

static PyObject *
listiter_next(listiterobject *it) {
  PyListObject *seq;
  PyObject *item;

  assert(it != NULL);
  seq = it->it_seq;
  if (seq == NULL)
    return NULL;
  assert(PyList_Check(seq));

  if (it->it_index < PyList_GET_SIZE(seq)) {
    item = PyList_GET_ITEM(seq, it->it_index);
    ++it->it_index;
    Py_INCREF(item);
    return item;
  }
  it->it_seq = NULL;
  Py_DECREF(seq);
  return NULL;
}

PyTypeObject PyListIter_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "list_iterator",
  .tp_basicsize = sizeof(listiterobject),
  .tp_dealloc = (destructor) listiter_dealloc,
  .tp_getattro = PyObject_GenericGetAttr,
  .tp_flags = 0,
  .tp_iter = PyObject_SelfIter,
  .tp_iternext = (iternextfunc) listiter_next,
  .tp_methods = listiter_methods,
};

static PyObject *
list_iter(PyObject *seq) {
  listiterobject *it;

  if (!PyList_Check(seq)) {
    fail(0);
  }
  it = PyObject_GC_New(listiterobject, &PyListIter_Type);
  if (it == NULL)
    return NULL;
  it->it_index = 0;
  Py_INCREF(seq);
  it->it_seq = (PyListObject *) seq;
  return (PyObject *) it;
}

// defined in cpy/Objects/listobject.c
PyTypeObject PyList_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "list",
  .tp_basicsize = sizeof(PyListObject),
  .tp_flags = Py_TPFLAGS_LIST_SUBCLASS,
	.tp_dealloc = (destructor) list_dealloc,
	.tp_free = PyObject_GC_Del,
	.tp_as_sequence = &list_as_sequence,
	.tp_as_mapping = &list_as_mapping,
	.tp_repr = (reprfunc) list_repr,
	.tp_methods = list_methods,
	.tp_getattro = PyObject_GenericGetAttr,
  .tp_iter = list_iter,
};

static PyObject *
list_new_prealloc(Py_ssize_t size) {
	assert(size > 0);
	PyListObject *op = (PyListObject *) PyList_New(0);
	if (op == NULL) {
		return NULL;
	}
	assert(op->ob_item == NULL);
	op->ob_item = PyMem_New(PyObject *, size);
	if (op->ob_item == NULL) {
		assert(false);
	}
	op->allocated = size;
	return (PyObject *) op;
}

static PyObject *
list_slice(PyListObject *a, Py_ssize_t ilow, Py_ssize_t ihigh) {
	PyListObject *np;
	PyObject **src, **dest;
	Py_ssize_t i, len;
	len = ihigh - ilow;

	if (len <= 0) {
		return PyList_New(0);
	}
	np = (PyListObject *) list_new_prealloc(len);
	if (np == NULL)
		return NULL;
	
	src = a->ob_item + ilow;
	dest = np->ob_item;
	for (i = 0; i < len; i++) {
		PyObject *v = src[i];
		Py_INCREF(v);
		dest[i] = v;
	}
	Py_SET_SIZE(np, len);
	return (PyObject *) np;
}

static PyObject *list_subscript(PyListObject *self, PyObject *item) {
	if (_PyIndex_Check(item)) {
		Py_ssize_t i;
		// TODO in cpy the second arg is PyExc_IndexError
		i = PyNumber_AsSsize_t(item, NULL);
		if (i == -1 && PyErr_Occurred())
			return NULL;
		if (i < 0)
			i += PyList_GET_SIZE(self);
		return list_item(self, i);
	} else if (PySlice_Check(item)) {
		Py_ssize_t start, stop, step, slicelength;

		if (PySlice_Unpack(item, &start, &stop, &step) < 0) {
			return NULL;
		}
		slicelength = PySlice_AdjustIndices(Py_SIZE(self), &start, &stop, step);
		if (slicelength <= 0) {
			return PyList_New(0);
		} else if (step == 1) {
			return list_slice(self, start, stop);
		} else {
			assert(false);
		}
	} else {
		assert(false);
	}
}

static int list_ass_subscript(PyListObject *self, PyObject *item, PyObject *value) {
	if (_PyIndex_Check(item)) {
		// TODO cpy pass the second arg as PyExc_IndexError
		Py_ssize_t i = PyNumber_AsSsize_t(item, NULL);
		if (i == -1 && PyErr_Occurred())
			return -1;
		if (i < 0)
			i += PyList_GET_SIZE(self);
		return list_ass_item(self, i, value);
	} else {
		assert(false);
	}
}
