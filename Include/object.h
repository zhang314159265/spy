#pragma once

#include <assert.h>
#include <string.h>
#include "sutil.h"
#include "pyport.h"
#include "cpython/objimpl.h"

// result of calling PyIter_Send
typedef enum {
  PYGEN_RETURN = 0,
  PYGEN_ERROR = -1,
  PYGEN_NEXT = 1,
} PySendResult;

#define Py_TPFLAGS_DISALLOW_INSTANTIATION (1UL << 7)

#define Py_TPFLAGS_IMMUTABLETYPE (1UL << 8)

// Set if the type object is dynamically allocated
#define Py_TPFLAGS_HEAPTYPE (1UL << 9)

// set if the type allows subclassing
#define Py_TPFLAGS_BASETYPE (1UL << 10)

#define Py_TPFLAGS_HAVE_VECTORCALL (1UL << 11)

// set if the type is 'ready' -- fully initialized
#define Py_TPFLAGS_READY (1UL << 12)

// set while the type is being 'readied', to prevent recursive ready calls
#define Py_TPFLAGS_READYING (1UL << 13)

#define Py_TPFLAGS_HAVE_GC (1UL << 14)
#define Py_TPFLAGS_METHOD_DESCRIPTOR (1UL << 17)

#define Py_TPFLAGS_VALID_VERSION_TAG (1UL << 19)
#define Py_TPFLAGS_IS_ABSTRACT (1UL << 20)

#define Py_TPFLAGS_LONG_SUBCLASS (1UL << 24)
#define Py_TPFLAGS_LIST_SUBCLASS (1UL << 25)
#define Py_TPFLAGS_TUPLE_SUBCLASS (1UL << 26)
#define Py_TPFLAGS_BYTES_SUBCLASS (1UL << 27)
#define Py_TPFLAGS_UNICODE_SUBCLASS (1UL << 28)
#define Py_TPFLAGS_DICT_SUBCLASS (1UL << 29)
#define Py_TPFLAGS_BASE_EXC_SUBCLASS (1UL << 30)
#define Py_TPFLAGS_TYPE_SUBCLASS (1UL << 31)

#define Py_TPFLAGS_HAVE_STACKLESS_EXTENSION 0

#define Py_TPFLAGS_DEFAULT ( \
  Py_TPFLAGS_HAVE_STACKLESS_EXTENSION | \
  0)

typedef struct _typeobject PyTypeObject;

typedef struct _object {
	Py_ssize_t ob_refcnt;
	PyTypeObject *ob_type;
} PyObject;

typedef int (*objobjproc)(PyObject *, PyObject *);
typedef int (*inquiry)(PyObject *);
typedef PyObject *(*unaryfunc)(PyObject *);
typedef PyObject * (*binaryfunc)(PyObject *, PyObject *);
typedef PyObject * (*ternaryfunc)(PyObject *, PyObject *, PyObject *);
typedef PyObject *(*getiterfunc)(PyObject *);
typedef PyObject *(*iternextfunc)(PyObject *);
typedef void (*destructor)(PyObject *);
typedef void (*freefunc)(void *);
typedef Py_hash_t (*hashfunc)(PyObject *);
typedef PyObject *(*richcmpfunc)(PyObject*, PyObject*, int);
typedef int (*setattrfunc)(PyObject *, char *, PyObject *);
typedef int(*setattrofunc)(PyObject *, PyObject *, PyObject *);
typedef int(*ssizeobjargproc)(PyObject *, Py_ssize_t, PyObject *);
typedef PyObject *(*getattrofunc)(PyObject *, PyObject *);
typedef PyObject *(*getattrfunc)(PyObject *, char *);
typedef Py_ssize_t (*lenfunc)(PyObject *);
typedef PyObject *(*ssizeargfunc)(PyObject *, Py_ssize_t);
typedef PyObject *(*reprfunc)(PyObject *);
typedef int(*objobjargproc)(PyObject *, PyObject *, PyObject *);
typedef PyObject *(*descrgetfunc)(PyObject *, PyObject *, PyObject *);
typedef int (*descrsetfunc)(PyObject *, PyObject *, PyObject *);
typedef PyObject *(*newfunc)(PyTypeObject *, PyObject *, PyObject *);
typedef int (*initproc)(PyObject *, PyObject *, PyObject *);
typedef int (*visitproc)(PyObject *, void *);
typedef int(*traverseproc)(PyObject *, visitproc, void *);


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
extern PyTypeObject PyType_Type;

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

// Generic type check
// defined in cpy/Objects/typeobject.c
int PyType_IsSubtype(PyTypeObject *a, PyTypeObject *b);

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

static inline int _PyObject_TypeCheck(PyObject *ob, PyTypeObject *type) {
	return Py_IS_TYPE(ob, type) || PyType_IsSubtype(Py_TYPE(ob), type);
}

#define PyObject_TypeCheck(ob, type) _PyObject_TypeCheck(_PyObject_CAST(ob), type)

int PyObject_SetAttrString(PyObject *v, const char *name, PyObject *w);

int PyObject_GenericSetAttr(PyObject *obj, PyObject *name, PyObject *value);

int
PyCallable_Check(PyObject *x) {
	if (x == NULL)
		return 0;
	return Py_TYPE(x)->tp_call != NULL;
}

#define Py_RETURN_NONE return Py_NewRef(Py_None)

PyObject *PyObject_GenericGetAttr(PyObject *obj, PyObject *name);
int _PyObject_GetMethod(PyObject *obj, PyObject *name, PyObject **method);

int Py_Is(PyObject *x, PyObject *y) {
	return x == y;
}

PyObject *PyObject_Str(PyObject *v);
PyObject *PyObject_Repr(PyObject *v);
int PyObject_IsTrue(PyObject *v);

#define Py_IsNone(x) Py_Is((x), Py_None)

Py_hash_t
PyObject_HashNotImplemented(PyObject *v) {
  assert(false);
}

int PyObject_SetAttr(PyObject *v, PyObject *name, PyObject *value);

PyObject *PyObject_SelfIter(PyObject *obj) {
  Py_INCREF(obj);
  return obj;
}

int PyObject_CallFinalizerFromDealloc(PyObject *self);

static inline int _PyType_CheckExact(PyObject *op) {
  return Py_IS_TYPE(op, &PyType_Type);
}

#define PyType_CheckExact(op) _PyType_CheckExact(_PyObject_CAST(op))

int PyObject_GenericSetDict(PyObject *obj, PyObject *value, void *context);

#define PyType_SUPPORTS_WEAKREFS(t) ((t)->tp_weaklistoffset > 0)

static inline PyObject **
_PyObject_GET_WEAKREFS_LISTPTR(PyObject *op) {
  Py_ssize_t offset = Py_TYPE(op)->tp_weaklistoffset;
  return (PyObject **) ((char *) op + offset);
}

PyObject *
PyObject_GetAttrString(PyObject *v, const char *name);

void PyErr_Fetch(PyObject **p_type, PyObject **p_value, PyObject **p_traceback);
void PyErr_Restore(PyObject *type, PyObject *value, PyObject *traceback);

typedef struct {
  int slot;
  void *pfunc;
} PyType_Slot;

typedef struct {
  const char *name;
  int basicsize;
  int itemsize;
  unsigned int flags;
  PyType_Slot *slots;
} PyType_Spec;

PyObject *PyType_FromSpec(PyType_Spec*);

PyObject *PyObject_Bytes(PyObject *v);

int PyObject_HasAttr(PyObject *v, PyObject *name);

int PyObject_CallFinalizerFromDealloc(PyObject *self);
