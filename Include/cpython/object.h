#pragma once

typedef PyObject *(*allocfunc)(PyTypeObject *, Py_ssize_t);

typedef PyObject *(*vectorcallfunc)(PyObject *callable, PyObject *const *args, size_t nargsf, PyObject *kwnames);

typedef struct {
  lenfunc mp_length;
  binaryfunc mp_subscript;
  objobjargproc mp_ass_subscript;
} PyMappingMethods;

typedef struct {
  binaryfunc nb_or;
  binaryfunc nb_xor;
  binaryfunc nb_inplace_or;
  binaryfunc nb_add;
  binaryfunc nb_inplace_add;
  binaryfunc nb_subtract;
  binaryfunc nb_floor_divide;
  binaryfunc nb_remainder;
  binaryfunc nb_true_divide;
  unaryfunc nb_absolute;
  ternaryfunc nb_power;
  binaryfunc nb_multiply;
	binaryfunc nb_and;
  binaryfunc nb_lshift;
  binaryfunc nb_rshift;

  unaryfunc nb_positive;
  unaryfunc nb_negative;
  unaryfunc nb_invert;
  inquiry nb_bool;

  binaryfunc nb_inplace_and;
  binaryfunc nb_inplace_xor;
  binaryfunc nb_inplace_subtract;
  binaryfunc nb_inplace_multiply;
  binaryfunc nb_inplace_remainder;
  binaryfunc nb_inplace_lshift;
  binaryfunc nb_inplace_rshift;
  binaryfunc nb_inplace_floor_divide;
  binaryfunc nb_inplace_true_divide;
  ternaryfunc nb_inplace_power;

  unaryfunc nb_index;

  unaryfunc nb_int;
  unaryfunc nb_float;
  binaryfunc nb_matrix_multiply;
  binaryfunc nb_inplace_matrix_multiply;
  binaryfunc nb_divmod;
} PyNumberMethods;

typedef struct {
  ssizeobjargproc sq_ass_item;
  binaryfunc sq_concat;
  lenfunc sq_length;
  ssizeargfunc sq_item;
  objobjproc sq_contains;
  ssizeargfunc sq_repeat;
  ssizeargfunc sq_inplace_repeat;
  binaryfunc sq_inplace_concat;
} PySequenceMethods;

typedef PySendResult (*sendfunc)(PyObject *iter, PyObject *value, PyObject **result);

typedef struct {
  unaryfunc am_await;
  unaryfunc am_aiter;
  unaryfunc am_anext;
  sendfunc am_send;
} PyAsyncMethods;

typedef struct bufferinfo {
  void *buf;
  PyObject *obj;
  Py_ssize_t len;
  Py_ssize_t itemsize;
  int readonly;
  int ndim;
  char *format;
  Py_ssize_t *shape;
  Py_ssize_t *strides;
  Py_ssize_t *suboffsets;
  void *internal;
} Py_buffer;

#define PyBUF_SIMPLE 0
#define PyBUF_WRITABLE 0x0001

#define PyBUF_FORMAT 0x0004
#define PyBUF_ND 0x0008
#define PyBUF_STRIDES (0x0010 | PyBUF_ND)

typedef int(*getbufferproc)(PyObject *, Py_buffer *, int);
typedef void (*releasebufferproc)(PyObject *, Py_buffer *);

typedef struct {
  getbufferproc bf_getbuffer;
  releasebufferproc bf_releasebuffer;
} PyBufferProcs;

struct PyMemberDef;

struct _typeobject {
	PyObject_VAR_HEAD
	const char *tp_name; // for printing, in format "<module>.<name>"
  Py_ssize_t tp_basicsize, tp_itemsize;
	unsigned long tp_flags;

  destructor tp_dealloc;

  allocfunc tp_alloc;
  inquiry tp_is_gc;
  destructor tp_del;

  PyObject *tp_mro; // method resolution order
  PyNumberMethods *tp_as_number;
  PyMappingMethods *tp_as_mapping;

  PyAsyncMethods *tp_as_async;
  PyBufferProcs *tp_as_buffer;

  struct _typeobject *tp_base;

  getiterfunc tp_iter;
  iternextfunc tp_iternext;
  freefunc tp_free; // Low-level free-memory routine

  PyObject *tp_dict;
  PyObject *tp_bases;

  hashfunc tp_hash;

  richcmpfunc tp_richcompare;

  setattrfunc tp_setattr;

  setattrofunc tp_setattro;

  Py_ssize_t tp_dictoffset;
  ternaryfunc tp_call;

  Py_ssize_t tp_vectorcall_offset;

  PySequenceMethods *tp_as_sequence;
  vectorcallfunc tp_vectorcall;

  getattrfunc tp_getattr;
  getattrofunc tp_getattro;
  struct PyMethodDef *tp_methods;

  reprfunc tp_repr;
  reprfunc tp_str;

  descrgetfunc tp_descr_get;
  descrsetfunc tp_descr_set;

  newfunc tp_new;
  initproc tp_init;

  // weak reference enabler
  Py_ssize_t tp_weaklistoffset;

  // delete references to contained objects
  inquiry tp_clear;

  traverseproc tp_traverse;
  struct PyMemberDef *tp_members;
  struct PyGetSetDef *tp_getset;

  destructor tp_finalize;

  const char *tp_doc;

  PyObject *tp_subclasses;
};

// The *real* layout of a type object when allocated on the heap
typedef struct _heaptypeobject {
  PyTypeObject ht_type;

  PyAsyncMethods as_async;
  PyNumberMethods as_number;
  PyMappingMethods as_mapping;
  PySequenceMethods as_sequence;

  PyBufferProcs as_buffer;
  PyObject *ht_name, *ht_slots, *ht_qualname;
  struct _dictkeysobject *ht_cached_keys;
  PyObject *ht_module;
} PyHeapTypeObject;

#define PyHeapType_GET_MEMBERS(etype) \
  ((struct PyMemberDef *)(((char *)etype) + Py_TYPE(etype)->tp_basicsize))

// defined in cpy/Objects/object.c
PyObject *_PyObject_NextNotImplemented(PyObject *self) {
  assert(false);
}

typedef struct _Py_Identifier {
  const char *string;
  Py_ssize_t index;
} _Py_Identifier;

#define _Py_static_string_init(value) { .string = value, .index = -1}

#define _Py_static_string(varname, value) static _Py_Identifier varname = _Py_static_string_init(value)
#define _Py_IDENTIFIER(varname) _Py_static_string(PyId_##varname, #varname)

#define Py_SETREF(op, op2) \
  do { \
    PyObject *_py_tmp = _PyObject_CAST(op); \
    (op) = (op2); \
    Py_DECREF(_py_tmp); \
  } while (0)

#define Py_XSETREF(op, op2) \
  do { \
    PyObject *_py_tmp = _PyObject_CAST(op); \
    (op) = (op2); \
    Py_XDECREF(_py_tmp); \
  } while (0)

int _PyObject_LookupAttrId(PyObject *v, struct _Py_Identifier *name, PyObject **result);


int _PyObject_LookupAttr(PyObject *v, PyObject *name, PyObject **result);
