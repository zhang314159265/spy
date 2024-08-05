#pragma once

#include "internal/pycore_object.h"
#include "tupleobject.h"

// Returns the result of bitwise or of o1 and o2, possibly in-place,
// or NULL on failure.
//
// This is the equivalent of the Python expression: o1 |= o2.
// (Shunting: can be used for set)
PyObject *PyNumber_InPlaceOr(PyObject *o1, PyObject *o2);

#define NB_BINOP(nb_methods, slot) \
    (*(binaryfunc*) (&((char*) nb_methods)[slot]))

static PyObject *
binary_iop1(PyObject *v, PyObject *w, const int iop_slot, const int op_slot
#ifndef NDEBUG
    , const char *op_name
#endif
    )
{
  PyNumberMethods *mv = Py_TYPE(v)->tp_as_number;
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
  assert(false);
}

#ifdef NDEBUG
#define BINARY_IOP1(v, w, iop_slot, op_slot, op_name) binary_iop1(v, w, iop_slot, op_slot)
#else
#define BINARY_IOP1(v, w, iop_slot, op_slot, op_name) binary_iop1(v, w, iop_slot, op_slot, op_name)
#endif

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
