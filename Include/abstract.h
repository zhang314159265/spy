#pragma once

#include "internal/pycore_object.h"
#include "tupleobject.h"

#define PY_VECTORCALL_ARGUMENTS_OFFSET ((size_t) 1 << (8 * sizeof(size_t) - 1))

// Returns the result of bitwise or of o1 and o2, possibly in-place,
// or NULL on failure.
//
// This is the equivalent of the Python expression: o1 |= o2.
// (Shunting: can be used for set)
PyObject *PyNumber_InPlaceOr(PyObject *o1, PyObject *o2);

#define NB_BINOP(nb_methods, slot) \
    (*(binaryfunc*) (&((char*) nb_methods)[slot]))

static PyObject *
binary_op1(PyObject *v, PyObject *w, const int op_slot) {
	// printf("binary_op1, %s v.s. %s\n", Py_TYPE(v)->tp_name, Py_TYPE(w)->tp_name);
	binaryfunc slotv;
	if (Py_TYPE(v)->tp_as_number != NULL) {
		slotv = NB_BINOP(Py_TYPE(v)->tp_as_number, op_slot);
	} else {
		slotv = NULL;
	}

	binaryfunc slotw;
	if (!Py_IS_TYPE(w, Py_TYPE(v)) && Py_TYPE(w)->tp_as_number != NULL) {
		slotw = NB_BINOP(Py_TYPE(w)->tp_as_number, op_slot);
		if (slotw == slotv) {
			slotw = NULL;
		}
	} else {
		slotw = NULL;
	}

	if (slotv) {
		PyObject *x;
		if (slotw && PyType_IsSubtype(Py_TYPE(w), Py_TYPE(v))) {
			assert(false);
		}
		x = slotv(v, w);
		assert(x != NULL);
		if (x != Py_NotImplemented) {
			return x;
		}
		Py_DECREF(x);
	}
	if (slotw) {
		PyObject *x = slotw(v, w);
		assert(x != NULL);
		if (x != Py_NotImplemented) {
			return x;
		}
		Py_DECREF(x);
	}
	Py_RETURN_NOTIMPLEMENTED;
}

static PyObject *
binary_iop1(PyObject *v, PyObject *w, const int iop_slot, const int op_slot
#ifndef NDEBUG
    , const char *op_name
#endif
    )
{
  PyNumberMethods *mv = Py_TYPE(v)->tp_as_number;
	// printf("v type %s\n", Py_TYPE(v)->tp_name);
  if (mv != NULL) {
    binaryfunc slot = NB_BINOP(mv, iop_slot);
    if (slot) {
      PyObject *x = (slot)(v, w);
      assert(_Py_CheckSlotResult(v, op_name, x != NULL));
      if (x != Py_NotImplemented) {
        return x;
      }
      Py_DECREF(x);
    }
  }
	return binary_op1(v, w, op_slot);
}

#ifdef NDEBUG
#define BINARY_IOP1(v, w, iop_slot, op_slot, op_name) binary_iop1(v, w, iop_slot, op_slot)
#else
#define BINARY_IOP1(v, w, iop_slot, op_slot, op_name) binary_iop1(v, w, iop_slot, op_slot, op_name)
#endif

#define BINARY_OP1(v, w, op_slot, op_name) binary_op1(v, w, op_slot)

static PyObject *
binary_iop(PyObject *v, PyObject *w, const int iop_slot, const int op_slot,
    const char *op_name) {
  PyObject *result = BINARY_IOP1(v, w, iop_slot, op_slot, op_name);
  if (result == Py_NotImplemented) {
    assert(false);
  }
  return result;
}

#define NB_SLOT(x) offsetof(PyNumberMethods, x)

// defined in cpy/Objects/abstract.c
#define INPLACE_BINOP(func, iop, op, op_name) \
  PyObject * \
  func(PyObject *v, PyObject *w) { \
    return binary_iop(v, w, NB_SLOT(iop), NB_SLOT(op), op_name); \
  }

INPLACE_BINOP(PyNumber_InPlaceOr, nb_inplace_or, nb_or, "|=")

// defined in cpy/Objects/abstract.c
int PyIter_Check(PyObject *obj) {
  PyTypeObject *tp = Py_TYPE(obj);
  return (tp->tp_iternext != NULL &&
    tp->tp_iternext != &_PyObject_NextNotImplemented);
}

PyObject *PyObject_GetIter(PyObject * o) {
  PyTypeObject *t = Py_TYPE(o);
  getiterfunc f;

  f = t->tp_iter;
  if (f == NULL) {
		printf("type %s has no tp_iter\n", t->tp_name);
    assert(false);
  } else {
    PyObject *res = (*f)(o);
    if (res != NULL && !PyIter_Check(res)) {
      assert(false);
    }
    return res;
  }
}

PyObject *
PyIter_Next(PyObject *iter) {
  PyObject *result;
  result = (*Py_TYPE(iter)->tp_iternext)(iter);
  if (result == NULL) {
    printf("WARNING: Setup exception when PyIter_Next returns NULL\n");
  }
  return result;
}

PyObject *PySequence_Tuple(PyObject *v) {
  if (v == NULL) {
    assert(false);
  }

  // Special-case the common tuple and list cases, for efficiency
  if (PyTuple_CheckExact(v)) {
    Py_INCREF(v);
    return v;
  }
  if (PyList_CheckExact(v)) {
    return PyList_AsTuple(v);
  }
  assert(false);
}

static inline vectorcallfunc
PyVectorcall_Function(PyObject *callable) {
	PyTypeObject *tp;
	Py_ssize_t offset;
	vectorcallfunc ptr;

	assert(callable != NULL);
	tp = Py_TYPE(callable);
	// printf("PyVectorcall_Function got type %s\n", tp->tp_name);
	if (!PyType_HasFeature(tp, Py_TPFLAGS_HAVE_VECTORCALL)) {
		return NULL;
	}
	assert(PyCallable_Check(callable));
	offset = tp->tp_vectorcall_offset;
	assert(offset > 0);
	memcpy(&ptr, (char *) callable + offset, sizeof(ptr));
	return ptr;
}


PyObject *_Py_CheckFunctionResult(PyThreadState *tstate, PyObject *callabble, PyObject *result, const char *where);

// defined in cpy/Include/cpython/abstract.h
static inline PyObject *
_PyObject_VectorcallTstate(PyThreadState *tstate, PyObject *callable,
		PyObject *const *args, size_t nargsf,
		PyObject *kwnames) {
	vectorcallfunc func;
	PyObject *res;

	func = PyVectorcall_Function(callable);
	if (func == NULL) {
		// printf("tp_name %s, no vector call\n", ((PyTypeObject *) Py_TYPE(callable))->tp_name);
		assert(false);
	}
	res = func(callable, args, nargsf, kwnames);
	return _Py_CheckFunctionResult(tstate, callable, res, NULL);
}

static inline PyObject *
PyObject_Vectorcall(PyObject *callable, PyObject *const *args,
		size_t nargsf, PyObject *kwnames) {
	PyThreadState *tstate = PyThreadState_Get();
	return _PyObject_VectorcallTstate(tstate, callable,
			args, nargsf, kwnames);
}

static inline Py_ssize_t
PyVectorcall_NARGS(size_t n) {
	return n & ~PY_VECTORCALL_ARGUMENTS_OFFSET;
}

int
PySequence_DelItem(PyObject *s, Py_ssize_t i) {
	printf("PySequence_DelItem type is %s\n", Py_TYPE(s)->tp_name);
	if (s == NULL) {
		assert(false);
	}

	PySequenceMethods *m = Py_TYPE(s)->tp_as_sequence;
	if (m && m->sq_ass_item) {
		if (i < 0) {
			assert(false);
		}
		int res = m->sq_ass_item(s, i, (PyObject *) NULL);
		assert(res >= 0);
		return res;
	}
	assert(false);
}

PyObject *
PyNumber_InPlaceAdd(PyObject *v, PyObject *w) {
	PyObject *result = BINARY_IOP1(v, w, NB_SLOT(nb_inplace_add),
			NB_SLOT(nb_add), "+=");
	if (result == Py_NotImplemented) {
		assert(false);
	}
	return result;
}

PyObject *
PyNumber_Add(PyObject *v, PyObject *w) {
	PyObject *result = BINARY_OP1(v, w, NB_SLOT(nb_add), "+");
	if (result != Py_NotImplemented) {
		return result;
	}
	assert(false);
}

PyObject *
_PyNumber_Index(PyObject *item) {
	if (item == NULL) {
		assert(false);
	}

	if (PyLong_Check(item)) {
		Py_INCREF(item);
		return item;
	}
	assert(false);
}

PyObject *
PyNumber_Index(PyObject *item) {
	PyObject *result = _PyNumber_Index(item);
	if (result != NULL && !PyLong_CheckExact(result)) {
		assert(false);
	}
	return result;
}

static PyObject *
binary_op(PyObject *v, PyObject *w, const int op_slot, const char *op_name) {
	PyObject *result = BINARY_OP1(v, w, op_slot, op_name);
	if (result == Py_NotImplemented) {
		printf("binary_op op name %s\n", op_name);
		assert(false);
	}
	return result;
}

#define BINARY_FUNC(func, op, op_name) \
	PyObject * \
	func(PyObject *v, PyObject *w) { \
		return binary_op(v, w, NB_SLOT(op), op_name); \
	}

BINARY_FUNC(PyNumber_Subtract, nb_subtract, "-")

PyObject *
PyNumber_FloorDivide(PyObject *v, PyObject *w) {
	return binary_op(v, w, NB_SLOT(nb_floor_divide), "//");
}

PyObject *
PySequence_GetItem(PyObject *s, Py_ssize_t i) {
	if (s == NULL) {
		assert(false);
	}

	printf("PySequence_GetItem got type %s\n", Py_TYPE(s)->tp_name);
	PySequenceMethods *m = Py_TYPE(s)->tp_as_sequence;
	if (m && m->sq_item) {
		if (i < 0) {
			assert(false);
		}
		PyObject *res = m->sq_item(s, i);
		assert(res != NULL);
		return res;
	}
	assert(false);
}

PyObject *
PyNumber_Remainder(PyObject *v, PyObject *w) {
	return binary_op(v, w, NB_SLOT(nb_remainder), "%");
}

PyObject *
PyNumber_TrueDivide(PyObject *v, PyObject *w) {
	return binary_op(v, w, NB_SLOT(nb_true_divide), "/");
}

PyObject *
PyNumber_Absolute(PyObject *o) {
	if (o == NULL) {
		assert(false);
	}
	PyNumberMethods *m = Py_TYPE(o)->tp_as_number;
	if (m && m->nb_absolute) {
		PyObject *res = m->nb_absolute(o);
		assert(res != NULL);
		return res;
	}
	assert(false);
}
