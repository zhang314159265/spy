#pragma once

#include <ffi.h>

#define FUNCFLAG_CDECL 0x1
#define FUNCFLAG_PYTHONAPI 0x4
#define FUNCFLAG_USE_ERRNO 0x8
#define FUNCFLAG_USE_LASTERROR 0x10

#define TYPEFLAG_ISPOINTER 0x100
#define DICTFLAG_FINAL 0x1000

typedef struct tagCDataObject CDataObject;
typedef struct tagPyCArgObject PyCArgObject;

union value {
  char c[16];
};

#define tagCDataObject_fields \
  char *b_ptr; \
  int b_needsfree; \
  CDataObject *b_base; \
  Py_ssize_t b_size; \
  Py_ssize_t b_length; \
  Py_ssize_t b_index; \
  PyObject *b_objects; \
  union value b_value;
  
struct tagCDataObject {
  PyObject_HEAD
  tagCDataObject_fields
};

struct tagPyCArgObject {
  PyObject_HEAD
  ffi_type *pffi_type;
  char tag;
  union {
    double d;
  } value;
  PyObject *obj;
  Py_ssize_t size;
};

typedef struct {
  PyObject_HEAD
  tagCDataObject_fields

  PyObject *callable;

  PyObject *converters;
  PyObject *argtypes;
  PyObject *restype;
  PyObject *checker;
  PyObject *errcheck;

  PyObject *paramflags;
} PyCFuncPtrObject;

typedef int(*PPROC)(void);

typedef PyObject *(*GETFUNC)(void *, Py_ssize_t size);
typedef PyObject *(*SETFUNC)(void *, PyObject *value, Py_ssize_t size);
typedef PyCArgObject *(*PARAMFUNC)(CDataObject *obj);

// A subclass of PyDictObject
typedef struct {
  PyDictObject dict;

  Py_ssize_t size;
  Py_ssize_t align;
  Py_ssize_t length;
  ffi_type ffi_type_pointer;
  PyObject *proto;
  SETFUNC setfunc;
  GETFUNC getfunc;
  PARAMFUNC paramfunc;

  PyObject *argtypes;  // tuple of CDataObjects
  PyObject *converters;
  PyObject *restype;
  PyObject *checker;

  int flags;

  char *format;
  int ndim;
  Py_ssize_t *shape;
} StgDictObject;

StgDictObject *PyObject_stgdict(PyObject *self);
StgDictObject *PyType_stgdict(PyObject *obj);


extern PyTypeObject PyCStgDict_Type;
#define PyCStgDict_CheckExact(v) Py_IS_TYPE(v, &PyCStgDict_Type)

struct fielddesc {
  char code;
  SETFUNC setfunc;
  GETFUNC getfunc;
  ffi_type *pffi_type; // statically allocated
  SETFUNC setfunc_swapped;
  GETFUNC getfunc_swapped;
};

extern PyTypeObject PyCArg_Type;
#define PyCArg_CheckExact(v) Py_IS_TYPE(v, &PyCArg_Type)

int _ctypes_simple_instance(PyObject *obj);

#define PyCSimpleTypeObject_Check(v) PyObject_TypeCheck(v, &PyCSimpleType_Type)

PyCArgObject *PyCArgObject_new(void);

#define _CDataObject_HasExternalBuffer(v) ((v)->b_ptr != (char *)&(v)->b_value)
