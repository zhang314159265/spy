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

#define NB_TERNOP(nb_methods, slot) \
		(*(ternaryfunc*)(&((char*) nb_methods)[slot]))

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
INPLACE_BINOP(PyNumber_InPlaceAnd, nb_inplace_and, nb_and, "&=")
INPLACE_BINOP(PyNumber_InPlaceXor, nb_inplace_xor, nb_xor, "^=")
INPLACE_BINOP(PyNumber_InPlaceSubtract, nb_inplace_subtract, nb_subtract, "-=")
INPLACE_BINOP(PyNumber_InPlaceLshift, nb_inplace_lshift, nb_lshift, "<<=")
INPLACE_BINOP(PyNumber_InPlaceRshift, nb_inplace_rshift, nb_rshift, ">>=")

PyObject *
PyNumber_InPlaceFloorDivide(PyObject *v, PyObject *w) {
	return binary_iop(v, w, NB_SLOT(nb_inplace_floor_divide),
			NB_SLOT(nb_floor_divide), "//=");
}

PyObject *
PyNumber_InPlaceTrueDivide(PyObject *v, PyObject *w) {
	return binary_iop(v, w, NB_SLOT(nb_inplace_true_divide),
		NB_SLOT(nb_true_divide), "/=");
}

PyObject *
PyNumber_InPlaceRemainder(PyObject *v, PyObject *w) {
	return binary_iop(v, w, NB_SLOT(nb_inplace_remainder),
		NB_SLOT(nb_remainder), "%=");
}

PyObject *
PyNumber_InPlaceMultiply(PyObject *v, PyObject *w) {
	PyObject *result = BINARY_IOP1(v, w, NB_SLOT(nb_inplace_multiply),
			NB_SLOT(nb_multiply), "*=");
	if (result == Py_NotImplemented) {
		assert(false);
	}
	return result;
}

static PyObject *ternary_op(PyObject *v,
		PyObject *w,
		PyObject *z,
		const int op_slot,
		const char *op_name);

static PyObject *
ternary_iop(PyObject *v, PyObject *w, PyObject *z, const int iop_slot, const int op_slot, const char *op_name) {
	PyNumberMethods *mv = Py_TYPE(v)->tp_as_number;
	if (mv != NULL) {
		ternaryfunc slot = NB_TERNOP(mv, iop_slot);
		if (slot) {
			assert(false);
		}
	}
	return ternary_op(v, w, z, op_slot, op_name);
}

PyObject *
PyNumber_InPlacePower(PyObject *v, PyObject *w, PyObject *z) {
	return ternary_iop(v, w, z, NB_SLOT(nb_inplace_power),
		NB_SLOT(nb_power), "**=");
}

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
  PyObject *it;
  Py_ssize_t n;
  PyObject *result = NULL;
  Py_ssize_t j;

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

  // Get iterator
  it = PyObject_GetIter(v);
  if (it == NULL) {
    return NULL;
  }

  n = PyObject_LengthHint(v, 10);
  if (n == -1)
    assert(false);
  result = PyTuple_New(n);
  if (result == NULL)
    assert(false);

  for (j = 0; ; ++j) {
    PyObject *item = PyIter_Next(it);
    if (item == NULL) {
      if (PyErr_Occurred())
        assert(false);
      break;
    }
    if (j >= n) {
      assert(false);
    }
    PyTuple_SET_ITEM(result, j, item);
  }
  if (j < n && _PyTuple_Resize(&result, j) != 0)
    assert(false);

  Py_DECREF(it);
  return result;
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

static inline Py_ssize_t
PyVectorcall_NARGS(size_t n) {
	return n & ~PY_VECTORCALL_ARGUMENTS_OFFSET;
}

PyObject *_PyObject_MakeTpCall(PyThreadState *tstate, PyObject *callable,
    PyObject *const *args, Py_ssize_t nargs,
    PyObject *keywords);

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
    Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
    return _PyObject_MakeTpCall(tstate, callable, args, nargs, kwnames);
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
  Py_DECREF(result);

  PySequenceMethods *m = Py_TYPE(v)->tp_as_sequence;
  if (m && m->sq_concat) {
    result = (*m->sq_concat)(v, w);
    assert(result != NULL);
    return result;
  }
	assert(false);
}

PyObject *_PyNumber_Index(PyObject *item);


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
BINARY_FUNC(PyNumber_And, nb_and, "&")
BINARY_FUNC(PyNumber_Or, nb_or, "|")
BINARY_FUNC(PyNumber_Xor, nb_xor, "^")
BINARY_FUNC(PyNumber_Lshift, nb_lshift, "<<")
BINARY_FUNC(PyNumber_Rshift, nb_rshift, ">>")

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

static PyObject *
ternary_op(PyObject *v,
		PyObject *w,
		PyObject *z,
		const int op_slot,
		const char *op_name) {
	PyNumberMethods *mv = Py_TYPE(v)->tp_as_number;
	PyNumberMethods *mw = Py_TYPE(w)->tp_as_number;

	ternaryfunc slotv;
	if (mv != NULL) {
		slotv = NB_TERNOP(mv, op_slot);
	} else {
		slotv = NULL;
	}

	ternaryfunc slotw;
	if (!Py_IS_TYPE(w, Py_TYPE(v)) && mw != NULL) {
		assert(false);
	} else {
		slotw = NULL;
	}

	if (slotv) {
		PyObject *x;
		if (slotw && PyType_IsSubtype(Py_TYPE(w), Py_TYPE(v))) {
			assert(false);
		}
		x = slotv(v, w, z);
		assert(x != NULL);
		if (x != Py_NotImplemented) {
			return x;
		}
		Py_DECREF(x);
	}
	assert(false);
}

PyObject *
PyNumber_Power(PyObject *v, PyObject *w, PyObject *z) {
	return ternary_op(v, w, z, NB_SLOT(nb_power), "** or pow()");
}

PyObject *
PyNumber_Multiply(PyObject *v, PyObject *w) {
	PyObject *result = BINARY_OP1(v, w, NB_SLOT(nb_multiply), "*");
	if (result == Py_NotImplemented) {
		assert(false);
	}
	return result;
}

PyObject *PySequence_Fast(PyObject *v, const char *m) {
	if (v == NULL) {
		assert(false);
	}

	if (PyList_CheckExact(v) || PyTuple_CheckExact(v)) {
		Py_INCREF(v);
		return v;
	}
	assert(false);
}

#define PySequence_Fast_GET_SIZE(o) \
	(PyList_Check(o) ? PyList_GET_SIZE(o) : PyTuple_GET_SIZE(o))

#define PySequence_Fast_ITEMS(sf) \
	(PyList_Check(sf) ? ((PyListObject *)(sf))->ob_item \
		: ((PyTupleObject *)(sf))->ob_item)

int PyObject_SetItem(PyObject *o, PyObject *key, PyObject *value) {
	if (o == NULL || key == NULL || value == NULL) {
		assert(false);
	}

	PyMappingMethods *m = Py_TYPE(o)->tp_as_mapping;
	if (m && m->mp_ass_subscript) {
		int res = m->mp_ass_subscript(o, key, value);
		assert(res >= 0);
		return res;
	}
	assert(false);
}

static inline int
_PyIndex_Check(PyObject *obj) {
	PyNumberMethods *tp_as_number = Py_TYPE(obj)->tp_as_number;
	return (tp_as_number != NULL && tp_as_number->nb_index != NULL);
}

Py_ssize_t
PyNumber_AsSsize_t(PyObject *item, PyObject *err);

PyObject *
PyObject_GetItem(PyObject *o, PyObject *key) {
	if (o == NULL || key == NULL) {
		assert(false);
	}

	PyMappingMethods *m = Py_TYPE(o)->tp_as_mapping;
	if (m && m->mp_subscript) {
		PyObject *item = m->mp_subscript(o, key);

    assert(_Py_CheckSlotResult(o, "__getitem__", item != NULL));
		return item;
	}

	// printf("PyObject_GetItem obj type %s, key type %s\n", Py_TYPE(o)->tp_name, Py_TYPE(key)->tp_name);
	PySequenceMethods *ms = Py_TYPE(o)->tp_as_sequence;
	if (ms && ms->sq_item) {
		if (_PyIndex_Check(key)) {
      Py_ssize_t key_value;
      // key_value = PyNumber_AsSsize_t(key, PyExc_IndexError);
      key_value = PyNumber_AsSsize_t(key, NULL);
      if (key_value == -1 && PyErr_Occurred())
        return NULL;

      return PySequence_GetItem(o, key_value);
		} else {
			assert(false);
		}
	}
	assert(false);
}

int PyObject_DelItem(PyObject *o, PyObject *key) {
  if (o == NULL || key == NULL) {
    fail(0);
  }

  PyMappingMethods *m = Py_TYPE(o)->tp_as_mapping;
  if (m && m->mp_ass_subscript) {
    int res = m->mp_ass_subscript(o, key, (PyObject*) NULL);
    assert(res >= 0);
    return res;
  }

  if (Py_TYPE(o)->tp_as_sequence) {
    fail(0);
  }
  fail(0);
  return -1;
}

PyObject *
PySequence_List(PyObject *v) {
	PyObject *result;
	PyObject *rv;

	if (v == NULL) {
		assert(false);
	}

	result = PyList_New(0);
	if (result == NULL)
		return NULL;
	
	rv = _PyList_Extend((PyListObject *) result, v);
	if (rv == NULL) {
		assert(false);
	}
	Py_DECREF(rv);
	return result;
}

Py_ssize_t 
PyObject_LengthHint(PyObject *o, Py_ssize_t defaultvalue) {
	// TODO follow cpy
	return defaultvalue;
	// assert(false);
}

PyObject *
PyNumber_Positive(PyObject *o) {
	if (o == NULL) {
		assert(false);
	}

	PyNumberMethods *m = Py_TYPE(o)->tp_as_number;
	if (m && m->nb_positive) {
		PyObject *res = (*m->nb_positive)(o);
		assert(res != NULL);
		return res;
	}
	assert(false);
}

PyObject *
PyNumber_Negative(PyObject *o) {
	if (o == NULL) {
		assert(false);
	}

	PyNumberMethods *m = Py_TYPE(o)->tp_as_number;
	if (m && m->nb_negative) {
		PyObject *res = (*m->nb_negative)(o);
		assert(res != NULL);
		return res;
	}
	assert(false);
}

PyObject *
PyNumber_Invert(PyObject *o) {
	if (o == NULL) {
		assert(false);
	}

	PyNumberMethods *m = Py_TYPE(o)->tp_as_number;
	if (m && m->nb_invert) {
		PyObject *res = (*m->nb_invert)(o);
		assert(res != NULL);
		return res;
	}
	assert(false);
}

int
PySequence_Contains(PyObject *seq, PyObject *ob) {
	PySequenceMethods *sqm = Py_TYPE(seq)->tp_as_sequence;
	if (sqm != NULL && sqm->sq_contains != NULL) {
		int res = (*sqm->sq_contains)(seq, ob);
		assert(res >= 0);
		return res;
	}
  printf("seq type %s\n", Py_TYPE(seq)->tp_name);
	assert(false);
}

// return an error on Overflow only if err is not NULL
Py_ssize_t
PyNumber_AsSsize_t(PyObject *item, PyObject *err) {
	Py_ssize_t result;
	PyObject *value = _PyNumber_Index(item);
	if (value == NULL)
		return -1;
	result = PyLong_AsSsize_t(value);
	if (result != -1)
		goto finish;
	assert(false);
finish:
	Py_DECREF(value);
	return result;
}

int
PyMapping_Check(PyObject *o) {
  return o && Py_TYPE(o)->tp_as_mapping &&
      Py_TYPE(o)->tp_as_mapping->mp_subscript;
}

static inline PyObject *
_PyObject_FastCallTstate(PyThreadState *tstate, PyObject *func, PyObject *const *args, Py_ssize_t nargs) {
  return _PyObject_VectorcallTstate(tstate, func, args, (size_t) nargs, NULL);
}

// defined in cpy/Include/cpython/abstract.h
static inline PyObject *
_PyObject_FastCall(PyObject *func, PyObject *const *args, Py_ssize_t nargs) {
  PyThreadState *tstate = PyThreadState_Get();
  return _PyObject_FastCallTstate(tstate, func, args, nargs);
}

PyObject *_PyObject_Call(PyThreadState *tstate, PyObject *callable, PyObject *args, PyObject *kwargs);

PyObject *PyObject_Call(PyObject *callable, PyObject *args, PyObject *kwargs);

#define _PY_FASTCALL_SMALL_STACK 5

PyObject *
PyObject_Type(PyObject *o) {
  PyObject *v;

  if (o == NULL) {
    assert(false);
  }

  v = (PyObject *) Py_TYPE(o);
  Py_INCREF(v);
  return v;
}

static inline PyObject *
PyObject_CallOneArg(PyObject *func, PyObject *arg) {
  PyObject *_args[2];
  PyObject **args;
  PyThreadState *tstate;
  size_t nargsf;

  assert(arg != NULL);
  args = _args + 1; // for PY_VECTORCALL_ARGUMENTS_OFFSET
  args[0] = arg;
  tstate = PyThreadState_Get();
  nargsf = 1 | PY_VECTORCALL_ARGUMENTS_OFFSET;
  return _PyObject_VectorcallTstate(tstate, func, args, nargsf, NULL);
}

static int
object_issubclass(PyThreadState *tstate, PyObject *derived, PyObject *cls) {
  if (PyType_CheckExact(cls)) {
    if (derived == cls)
      return 1;
    assert(false);
  }
  assert(false);
}

int
PyObject_IsSubclass(PyObject *derived, PyObject *cls) {
  PyThreadState *tstate = _PyThreadState_GET();
  return object_issubclass(tstate, derived, cls);
}

static inline PyObject *
_PyObject_CallNoArg(PyObject *func) {
  PyThreadState *tstate = PyThreadState_Get();
  return _PyObject_VectorcallTstate(tstate, func, NULL, 0, NULL);
}

int
PyMapping_SetItemString(PyObject *o, const char *key, PyObject *value) {
  PyObject *okey;
  int r;

  if (key == NULL) {
    fail(0);
  }

  okey = PyUnicode_FromString(key);
  if (okey == NULL)
    return -1;
  r = PyObject_SetItem(o, okey, value);
  Py_DECREF(okey);
  return r;
}

static int
object_isinstance(PyObject *inst, PyObject *cls) {
  PyObject *icls;
  int retval;
  _Py_IDENTIFIER(__class__);

  if (PyType_Check(cls)) {
    retval = PyObject_TypeCheck(inst, (PyTypeObject *) cls);
    if (retval == 0) {
      retval = _PyObject_LookupAttrId(inst, &PyId___class__, &icls);
      if (icls != NULL) {
        if (icls != (PyObject *)(Py_TYPE(inst)) && PyType_Check(icls)) {
          fail(0);
        } else {
          retval = 0;
        }
        Py_DECREF(icls);
      }
    }
  } else {
    fail(0);
  }

  return retval;
}

static int
object_recursive_isinstance(PyThreadState *tstate, PyObject *inst, PyObject *cls) {
  if (Py_IS_TYPE(inst, (PyTypeObject *) cls)) {
    return 1;
  }

  if (PyType_CheckExact(cls)) {
    return object_isinstance(inst, cls);
  }

  if (PyTuple_Check(cls)) {
    Py_ssize_t n = PyTuple_GET_SIZE(cls);
    int r = 0;
    for (Py_ssize_t i = 0; i < n; ++i) {
      PyObject *item = PyTuple_GET_ITEM(cls, i);
      r = object_recursive_isinstance(tstate, inst, item);
      if (r != 0) {
        break;
      }
    }
    return r;
  }
  fail(0);
}

int PyObject_IsInstance(PyObject *inst, PyObject *cls) {
  PyThreadState *tstate = _PyThreadState_GET();
  return object_recursive_isinstance(tstate, inst, cls);
}

Py_ssize_t
PyMapping_Size(PyObject *o) {
  fail(0);
}

Py_ssize_t 
PyObject_Size(PyObject *o) {
  if (o == NULL) {
    fail(0);
  }

  PySequenceMethods *m = Py_TYPE(o)->tp_as_sequence;
  if (m && m->sq_length) {
    Py_ssize_t len = m->sq_length(o);
    assert(len >= 0);
    return len;
  }
  return PyMapping_Size(o);
}

int
PyObject_GetBuffer(PyObject *obj, Py_buffer *view, int flags) {
  PyBufferProcs *pb = Py_TYPE(obj)->tp_as_buffer;

  if (pb == NULL || pb->bf_getbuffer == NULL) {
    fail(0);
  }
  int res = (*pb->bf_getbuffer)(obj, view, flags);
  assert(res >= 0);
  return res;
}

int PyBuffer_FillInfo(Py_buffer *view, PyObject *obj, void *buf, Py_ssize_t len, int readonly, int flags) {
  if (view == NULL) {
    fail(0);
  }

  if (((flags & PyBUF_WRITABLE) == PyBUF_WRITABLE) && (readonly == 1)) {
    fail(0);
  }
  view->obj = obj;
  if (obj)
    Py_INCREF(obj);
  view->buf = buf;
  view->len = len;
  view->readonly = readonly;
  view->itemsize = 1;
  view->format = NULL;
  if ((flags & PyBUF_FORMAT) == PyBUF_FORMAT) {
    fail(0);
  }
  view->ndim = 1;
  view->shape = NULL;
  if ((flags & PyBUF_ND) == PyBUF_ND) {
    fail(0);
  }
  view->strides = NULL;
  if ((flags & PyBUF_STRIDES) == PyBUF_STRIDES)
    fail(0);
  view->suboffsets = NULL;
  view->internal = NULL;
  return 0;
}

void PyBuffer_Release(Py_buffer *view) {
  PyObject *obj = view->obj;
  PyBufferProcs *pb;
  if (obj == NULL)
    return;
  pb = Py_TYPE(obj)->tp_as_buffer;
  if (pb && pb->bf_releasebuffer) {
    pb->bf_releasebuffer(obj, view);
  }
  view->obj = NULL;
  Py_DECREF(obj);
}

int
PySequence_Check(PyObject *s) {
  if (PyDict_Check(s))
    return 0;
  return Py_TYPE(s)->tp_as_sequence &&
    Py_TYPE(s)->tp_as_sequence->sq_item != NULL;
}

int PyIndex_Check(PyObject *obj) {
  return _PyIndex_Check(obj);
}

int
PyObject_CheckBuffer(PyObject *obj) {
  PyBufferProcs *tp_as_buffer = Py_TYPE(obj)->tp_as_buffer;
  return (tp_as_buffer != NULL && tp_as_buffer->bf_getbuffer != NULL);
}

int
PyNumber_Check(PyObject *o) {
  if (o == NULL)
    return 0;
  PyNumberMethods *nb = Py_TYPE(o)->tp_as_number;
  // return nb && (nb->nb_index || nb->nb_int || nb->nb_float || PyComplex_Check(o));
  return nb && (nb->nb_index || nb->nb_int || nb->nb_float);
}

PyObject *
PyNumber_Long(PyObject *o) {
  PyObject *result;
  PyNumberMethods *m;
  if (o == NULL) {
    fail(0);
  }

  if (PyLong_CheckExact(o)) {
    Py_INCREF(o);
    return o;
  }
  m = Py_TYPE(o)->tp_as_number;
  if (m && m->nb_int) {
    result = m->nb_int(o);
    assert(result != NULL);
    if (!result || PyLong_CheckExact(result)) {
      return result;
    }
    fail(0);
  }
  fail(0);
}

static inline PyObject *_PyObject_CallMethodIdNoArgs(PyObject *self, _Py_Identifier *name);

static inline PyObject *_PyObject_VectorcallMethodId(_Py_Identifier *name, PyObject *const *args, size_t nargsf, PyObject *kwnames);

PyObject *PyObject_VectorcallMethod(PyObject *name, PyObject *const *args, size_t nargsf, PyObject *kwnames);

static inline PyObject *
PyObject_CallMethodNoArgs(PyObject *self, PyObject *name) {
  return PyObject_VectorcallMethod(name, &self,
      1 | PY_VECTORCALL_ARGUMENTS_OFFSET, NULL);
}
