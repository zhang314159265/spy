#pragma once

#define PySlice_Check(op) Py_IS_TYPE(op, &PySlice_Type)

typedef struct {
	PyObject_HEAD
	PyObject *start, *stop, *step;
} PySliceObject;

static void slice_dealloc(PySliceObject *r);

PyTypeObject PySlice_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "slice",
	.tp_basicsize = sizeof(PySliceObject),
	.tp_dealloc = (destructor) slice_dealloc,
};

PyObject *
PySlice_New(PyObject *start, PyObject *stop, PyObject *step) {
	if (step == NULL) {
		step = Py_None;
	}
	if (start == NULL) {
		start = Py_None;
	}
	if (stop == NULL) {
		stop = Py_None;
	}

	PySliceObject *obj;
	obj = PyObject_GC_New(PySliceObject, &PySlice_Type);
	if (obj == NULL) {
		return NULL;
	}

	Py_INCREF(step);
	obj->step = step;
	Py_INCREF(start);
	obj->start = start;
	Py_INCREF(stop);
	obj->stop = stop;

	_PyObject_GC_TRACK(obj);
	return (PyObject *) obj;
}

int _PyEval_SliceIndex(PyObject *v, Py_ssize_t *pi);

int PySlice_Unpack(PyObject *_r,
		Py_ssize_t *start, Py_ssize_t *stop, Py_ssize_t *step) {
	PySliceObject *r = (PySliceObject *) _r;

	if (r->step == Py_None) {
		*step = 1;
	} else {
		assert(false);
	}

	if (r->start == Py_None) {
    *start = *step < 0 ? PY_SSIZE_T_MAX : 0;
	} else {
		if (!_PyEval_SliceIndex(r->start, start)) return -1;
	}

	if (r->stop == Py_None) {
    *stop = *step < 0 ? PY_SSIZE_T_MIN : PY_SSIZE_T_MAX;
	} else {
		if (!_PyEval_SliceIndex(r->stop, stop)) return -1;
	}

	return 0;
}

Py_ssize_t
PySlice_AdjustIndices(Py_ssize_t length,
		Py_ssize_t *start, Py_ssize_t *stop, Py_ssize_t step) {
	assert(step != 0);

	if (*start < 0) {
		assert(false);
	} else if (*start >= length) {
		assert(false);
	}

	if (*stop < 0) {
		assert(false);
	} else if (*stop >= length) {
    *stop = (step < 0) ? length - 1 : length;
	}

	if (step < 0) {
		assert(false);
	} else {
		if (*start < *stop) {
			return (*stop - *start - 1) / step + 1;
		}
	}
	return 0;
}

static void slice_dealloc(PySliceObject *r) {
	Py_DECREF(r->step);
	Py_DECREF(r->start);
	Py_DECREF(r->stop);
	PyObject_GC_Del(r);
}
