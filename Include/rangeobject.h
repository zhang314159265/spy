#pragma once

typedef struct {
	PyObject_HEAD
	PyObject *start;
	PyObject *stop;
	PyObject *step;
	PyObject *length;
} rangeobject;


static PyObject *
compute_range_length(PyObject *start, PyObject *stop, PyObject *step) {
	// Algorithm is equal to that of get_len_of_range(), but it operates
	// on PyObjects
	int cmp_result;
	PyObject *lo, *hi;
	PyObject *diff = NULL;
	PyObject *tmp1 = NULL, *tmp2 = NULL, *result;

	PyObject *zero = _PyLong_GetZero();
	PyObject *one = _PyLong_GetOne();

	cmp_result = PyObject_RichCompareBool(step, zero, Py_GT);
	if (cmp_result == -1)
		return NULL;
	
	if (cmp_result == 1) {
		lo = start;
		hi = stop;
		Py_INCREF(step);
	} else {
		assert(false);
	}

	cmp_result = PyObject_RichCompareBool(lo, hi, Py_GE);
	if (cmp_result != 0) {
		Py_DECREF(step);
		if (cmp_result < 0)
			return NULL;
		result = zero;
		Py_INCREF(result);
		return result;
	}

	if ((tmp1 = PyNumber_Subtract(hi, lo)) == NULL)
		assert(false);
	
	if ((diff = PyNumber_Subtract(tmp1, one)) == NULL)
		assert(false);
	
	if ((tmp2 = PyNumber_FloorDivide(diff, step)) == NULL)
		assert(false);
	
	if ((result = PyNumber_Add(tmp2, one)) == NULL)
		assert(false);
	
	Py_DECREF(tmp2);
	Py_DECREF(diff);
	Py_DECREF(step);
	Py_DECREF(tmp1);
	return result;
}

static rangeobject *
make_range_object(PyTypeObject *type, PyObject *start,
		PyObject *stop, PyObject *step) {
	rangeobject *obj = NULL;
	PyObject *length;
	length = compute_range_length(start, stop, step);
	if (length == NULL) {
		return NULL;
	}
	obj = PyObject_New(rangeobject, type);
	if (obj == NULL) {
		assert(false);
	}
	obj->start = start;
	obj->stop = stop;
	obj->step = step;
	obj->length = length;
	return obj;
}

static PyObject *
range_from_array(PyTypeObject *type, PyObject *const *args, Py_ssize_t num_args) {
	rangeobject *obj;
	PyObject *start = NULL, *stop = NULL, *step = NULL;

	switch (num_args) {
	case 1:
		stop = PyNumber_Index(args[0]);
		if (!stop) {
			return NULL;
		}
		start = _PyLong_GetZero();
		Py_INCREF(start);
		step = _PyLong_GetOne();
		Py_INCREF(step);
		break;
	default:
		assert(false);
	}
	obj = make_range_object(type, start, stop, step);
	if (obj != NULL) {
		return (PyObject *) obj;
	}

	Py_DECREF(start);
	Py_DECREF(stop);
	Py_DECREF(step);
	return NULL;
}

static PyObject *
range_vectorcall(PyTypeObject *type, PyObject *const *args,
		size_t nargsf, PyObject *kwnames) {
	Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
	// printf("range nargs %ld\n", nargs);
	return range_from_array(type, args, nargs);
}

static PyObject *range_iter(PyObject *seq);
static void range_dealloc(rangeobject *r);

PyTypeObject PyRange_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "range",
	.tp_basicsize = sizeof(rangeobject),
	.tp_vectorcall = (vectorcallfunc) range_vectorcall,
	.tp_iter = range_iter,
	.tp_dealloc = (destructor) range_dealloc,
};

#define PyRange_Check(op) Py_IS_TYPE(op, &PyRange_Type)

static unsigned long
get_len_of_range(long lo, long hi, long step) {
	assert(step != 0);
	if (step > 0 && lo < hi) {
		return 1UL + (hi - 1UL - lo) / step;
	} else if (step < 0 && lo > hi) {
		assert(false);
	} else {
		return 0UL;
	}
}

typedef struct {
	PyObject_HEAD
	long index;
	long start;
	long step;
	long len;
} rangeiterobject;

static PyObject *rangeiter_next(rangeiterobject *r);

PyTypeObject PyRangeIter_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "range_iterator",
	.tp_basicsize = sizeof(rangeiterobject),
	.tp_iternext = (iternextfunc) rangeiter_next,
	.tp_dealloc = (destructor) PyObject_Del,
};

static PyObject *rangeiter_next(rangeiterobject *r) {
	if (r->index < r->len) {
		return PyLong_FromLong((long)(r->start +
				(unsigned long)(r->index++) * r->step));
	}
	return NULL;
}

static PyObject *
fast_range_iter(long start, long stop, long step, long len) {
	rangeiterobject *it = PyObject_New(rangeiterobject, &PyRangeIter_Type);
	if (it == NULL)
		return NULL;
	it->start = start;
	it->step = step;
	it->len = len;
	it->index = 0;
	return (PyObject *) it;
}

static PyObject *range_iter(PyObject *seq) {
	rangeobject *r = (rangeobject *) seq;
	long lstart, lstop, lstep;
	unsigned long ulen;

	assert(PyRange_Check(seq));

	lstart = PyLong_AsLong(r->start);
	if (lstart == -1 && PyErr_Occurred()) {
		assert(false);
	}
	lstop = PyLong_AsLong(r->stop);
	if (lstop == -1 && PyErr_Occurred()) {
		assert(false);
	}
	lstep = PyLong_AsLong(r->step);
	if (lstep == -1 && PyErr_Occurred()) {
		assert(false);
	}
	ulen = get_len_of_range(lstart, lstop, lstep);
	if (ulen > (unsigned long) LONG_MAX) {
		assert(false);
	}
	// TODO: check for overflow
	return fast_range_iter(lstart, lstop, lstep, (long) ulen);
}

static void range_dealloc(rangeobject *r) {
	Py_DECREF(r->start);
	Py_DECREF(r->stop);
	Py_DECREF(r->step);
	Py_DECREF(r->length);
	PyObject_Free(r);
}
