#ifndef Py_OBJECT_H
#define Py_OBJECT_H

#include <assert.h>
#include <string.h>
#include "sutil.h"
#include "pyport.h"
#include "cpython/objimpl.h"

// Set if the type object is dynamically allocated
#define Py_TPFLAGS_HEAPTYPE (1UL << 9)

// set if the type is 'ready' -- fully initialized
#define Py_TPFLAGS_READY (1UL << 12)

// set while the type is being 'readied', to prevent recursive ready calls
#define Py_TPFLAGS_READYING (1UL << 13)

#define Py_TPFLAGS_HAVE_GC (1UL << 14)
#define Py_TPFLAGS_LONG_SUBCLASS (1UL << 24)
#define Py_TPFLAGS_LIST_SUBCLASS (1UL << 25)
#define Py_TPFLAGS_TUPLE_SUBCLASS (1UL << 26)
#define Py_TPFLAGS_BYTES_SUBCLASS (1UL << 27)
#define Py_TPFLAGS_UNICODE_SUBCLASS (1UL << 28)
#define Py_TPFLAGS_DICT_SUBCLASS (1UL << 29)
#define Py_TPFLAGS_TYPE_SUBCLASS (1UL << 31)

typedef struct _typeobject PyTypeObject;

typedef struct _object {
	Py_ssize_t ob_refcnt;
	PyTypeObject *ob_type;
} PyObject;

typedef PyObject * (*binaryfunc)(PyObject *, PyObject *);
typedef PyObject *(*getiterfunc)(PyObject *);
typedef PyObject *(*iternextfunc)(PyObject *);
typedef void (*destructor)(PyObject *);
typedef void (*freefunc)(void *);
typedef Py_hash_t (*hashfunc)(PyObject *);
typedef PyObject *(*richcmpfunc)(PyObject*, PyObject*, int);

typedef struct {
	PyObject ob_base;
	Py_ssize_t ob_size;
} PyVarObject;

void _Py_SET_SIZE(PyVarObject *ob, Py_ssize_t size) {
	ob->ob_size = size;
}

#define Py_SET_SIZE(ob, size) _Py_SET_SIZE((PyVarObject*) (ob), size)

void
_Py_SET_TYPE(PyObject *ob, PyTypeObject *type) {
	ob->ob_type = type;
}

#define Py_SET_TYPE(ob, type) _Py_SET_TYPE((PyObject*) ob, type);

void _Py_SET_REFCNT(PyObject *ob, Py_ssize_t refcnt) {
	ob->ob_refcnt = refcnt;
}

#define Py_SET_REFCNT(ob, refcnt) _Py_SET_REFCNT((PyObject *) ob, refcnt)

#define Py_TYPE(ob) (((PyObject *) (ob))->ob_type)

#define _PyVarObject_CAST(op) ((PyVarObject *) (op))

#define Py_SIZE(ob) (_PyVarObject_CAST(ob)->ob_size)

#define PyObject_HEAD PyObject ob_base;
#define PyObject_VAR_HEAD PyVarObject ob_base;

#define PyObject_HEAD_INIT(type)  \
	{ 1, type},

#define PyVarObject_HEAD_INIT(type, size) \
	{ PyObject_HEAD_INIT(type) size },

#include "cpython/object.h"

// defined in cpy/Objects/typeobject.c
PyTypeObject PyType_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "type",
  .tp_basicsize = sizeof(PyHeapTypeObject),
  .tp_flags = Py_TPFLAGS_TYPE_SUBCLASS,
};

void _Py_INCREF(PyObject *op) {
	op->ob_refcnt++;
}

void _Py_Dealloc(PyObject *op) {
  destructor dealloc = Py_TYPE(op)->tp_dealloc;
	if (!dealloc) {
    printf("call _Py_Dealloc on obj of type %s with NULL dealloc function\n", Py_TYPE(op)->tp_name);
	}
  (*dealloc)(op);
}

void _Py_DECREF(PyObject *op) {
  if (--op->ob_refcnt != 0) {
  } else {
    _Py_Dealloc(op);
  }
}

int
_Py_IS_TYPE(const PyObject *ob, const PyTypeObject *type) {
	return ob->ob_type == type;
}
#define Py_IS_TYPE(ob, type) _Py_IS_TYPE((const PyObject *) (ob), type)

#define Py_INCREF(op) _Py_INCREF((PyObject*) (op))
#define Py_DECREF(op) _Py_DECREF((PyObject*) (op))

static inline void _Py_XDECREF(PyObject *op) {
  if (op != NULL) {
    Py_DECREF(op);
  }
}
#define Py_XDECREF(op) _Py_XDECREF(_PyObject_CAST(op))

static inline void _Py_XINCREF(PyObject *op) {
  if (op != NULL) {
    Py_INCREF(op);
  }
}

#define Py_XINCREF(op) _Py_XINCREF(_PyObject_CAST(op))

#include "cpython/object.h"

int
PyType_HasFeature(PyTypeObject *type, unsigned long feature) {
	return ((type->tp_flags & feature) != 0);
}

#define PyType_FastSubclass(type, flag) PyType_HasFeature(type, flag)

#define _PyObject_CAST(op) ((PyObject *) (op))

static inline int _PyType_Check(PyObject *op) {
  return PyType_FastSubclass(Py_TYPE(op), Py_TPFLAGS_TYPE_SUBCLASS);
}

#define PyType_Check(op) _PyType_Check(_PyObject_CAST(op))

#include "internal/pycore_object.h"
#include "objimpl.h"

// defined in cpy/Objects/typeobject.c
PyObject *PyType_GenericAlloc(PyTypeObject *type, Py_ssize_t nitems) {
  PyObject *obj;
  const size_t size = _PyObject_VAR_SIZE(type, nitems + 1);
  /* note that we need to add one, for the sentinel */

  if (_PyType_IS_GC(type)) {
    obj = _PyObject_GC_Malloc(size);
  } else {
    assert(false);
  }
  
  if (obj == NULL) {
    assert(false);
  }

  memset(obj, '\0', size);

  if (type->tp_itemsize == 0) {
    _PyObject_Init(obj, type);
  } else {
    assert(false);
  }

  if (_PyType_IS_GC(type)) {
    _PyObject_GC_TRACK(obj);
  }
  return obj;
}

// Generic type check
// defined in cpy/Objects/typeobject.c
int PyType_IsSubtype(PyTypeObject *a, PyTypeObject *b) {
  PyObject *mro;

  mro = a->tp_mro;
  assert(false);
}

PyTypeObject _PyNotImplemented_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "NotImplementedType",
};

#define _PyObject_EXTRA_INIT

// defined in cpy/Objects/object.c
PyObject _Py_NotImplementedStruct = {
  _PyObject_EXTRA_INIT
  1, &_PyNotImplemented_Type
};
#define Py_NotImplemented (&_Py_NotImplementedStruct)

// defined in cpy/Objects/object.c
Py_hash_t PyObject_Hash(PyObject *v) {
  PyTypeObject *tp = Py_TYPE(v);
  if (tp->tp_hash != NULL)
    return (*tp->tp_hash)(v);
  printf("PyObject_Hash lack support for obj type %s\n", Py_TYPE(v)->tp_name);
  assert(false);
}

/* Rich comparison opcodes */
#define Py_LT 0
#define Py_LE 1
#define Py_EQ 2
#define Py_NE 3
#define Py_GT 4
#define Py_GE 5

PyObject * PyObject_RichCompare(PyObject *v, PyObject *w, int op);
int PyObject_RichCompareBool(PyObject *v, PyObject *w, int op);

static inline PyObject *_Py_NewRef(PyObject *obj) {
  Py_INCREF(obj);
  return obj;
}

#define Py_NewRef(obj) _Py_NewRef(_PyObject_CAST(obj))

#define Py_RETURN_NOTIMPLEMENTED return Py_NewRef(Py_NotImplemented)

#define Py_RETURN_RICHCOMPARE(val1, val2, op) \
  do { \
    switch (op) { \
    case Py_EQ: if ((val1) == (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE; \
    case Py_NE: if ((val1) != (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE; \
    case Py_LT: if ((val1) < (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE; \
    case Py_GT: if ((val1) > (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE; \
    case Py_LE: if ((val1) <= (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE; \
    case Py_GE: if ((val1) >= (val2)) Py_RETURN_TRUE; Py_RETURN_FALSE; \
    default: \
      Py_UNREACHABLE(); \
    } \
  } while (0)

PyTypeObject _PyNone_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "NoneType",
};

PyObject _Py_NoneStruct = {
  _PyObject_EXTRA_INIT
  1, &_PyNone_Type
};

#define Py_None (&_Py_NoneStruct)

static inline Py_ssize_t _Py_REFCNT(const PyObject *ob) {
  return ob->ob_refcnt;
}

#define _PyObject_CAST_CONST(op) ((const PyObject*)(op))
#define Py_REFCNT(ob) _Py_REFCNT(_PyObject_CAST_CONST(ob))

#define Py_CLEAR(op) \
	do { \
		PyObject *_py_tmp = _PyObject_CAST(op); \
		if (_py_tmp != NULL) { \
			(op) = NULL; \
			Py_DECREF(_py_tmp); \
		} \
	} while (0)

static inline PyObject *_Py_XNewRef(PyObject *obj) {
	Py_XINCREF(obj);
	return obj;
}

#define Py_XNewRef(obj) _Py_XNewRef(_PyObject_CAST(obj))

#endif
