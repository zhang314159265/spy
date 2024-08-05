#pragma once

void
_Py_NewReference(PyObject *op) {
	Py_SET_REFCNT(op, 1);
}

void
_PyObject_Init(PyObject *op, PyTypeObject *typeobj) {
	assert(op);
	Py_SET_TYPE(op, typeobj);
	// TODO may need increment the refcount for typeobj
	_Py_NewReference(op);
}

void
_PyObject_InitVar(PyVarObject *op, PyTypeObject *typeobj, Py_ssize_t size) {
	assert(op);
	Py_SET_SIZE(op, size);
	_PyObject_Init((PyObject *) op, typeobj);
}

static inline void _PyObject_GC_TRACK(
#ifndef NDEBUG
  const char *filename, int lineno,
#endif
  PyObject *op) {
  // TODO nop for now
}

static inline void _PyObject_GC_UNTRACK(
#ifndef NDEBUG
  const char *filename, int lineno,
#endif
  PyObject *op)
{
  // TODO nop for now
}

#ifdef NDEBUG
#error NDEBUG defined
#else
#define _PyObject_GC_TRACK(op) \
    _PyObject_GC_TRACK(__FILE__, __LINE__, _PyObject_CAST(op))
#define _PyObject_GC_UNTRACK(op) \
    _PyObject_GC_UNTRACK(__FILE__, __LINE__, _PyObject_CAST(op))
#endif

// Fast inlined version of PyType_HasFeature()
static inline int
_PyType_HasFeature(PyTypeObject *type, unsigned long feature) {
  return ((type->tp_flags & feature) != 0);
}

// Fast inlined version of PyType_IS_GC()
#define _PyType_IS_GC(t) _PyType_HasFeature((t), Py_TPFLAGS_HAVE_GC)

// defined in cpy/Objects/call.c
int _Py_CheckSlotResult(
    PyObject *obj,
    const char *slot_name,
    int success) {
  if (!success) {
    assert(false);
  } else {
  }
  return 1;
}

#define _PyType_IsReady(type) ((type)->tp_dict != NULL)
