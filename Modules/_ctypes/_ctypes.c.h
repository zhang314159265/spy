
#define ctypes_dlsym dlsym
#define ctypes_dlopen dlopen
#include <dlfcn.h>

#include "ctypes.h"


#include "cfield.c.h"
#include "callproc.c.h"
#include "stgdict.c.h"

static int _ctypes_add_types(PyObject *mod);

static struct PyModuleDef _ctypesmodule = {
  PyModuleDef_HEAD_INIT,
  .m_name = "_ctypes",
  .m_doc = "",
  .m_size = -1,
  .m_methods = _ctypes_module_methods,
};

static void
PyCFuncPtr_dealloc(PyCFuncPtrObject *self) {
  fail(0);
}

static PyObject *
_build_callargs(PyCFuncPtrObject *self, PyObject *argtypes,
    PyObject *inargs, PyObject *kwds,
    int *poutmask, int *pinoutmask, unsigned int *pnumretvals) {

  PyObject *paramflags = self->paramflags;

  *poutmask = 0;
  *pinoutmask = 0;
  *pnumretvals = 0;

  if (argtypes == NULL || paramflags == NULL || PyTuple_GET_SIZE(argtypes) == 0) {
    Py_INCREF(inargs);
    return inargs;
  }

  fail(0);
}

static PyObject *
_build_result(PyObject *result, PyObject *callargs,
    int outmask, int inoutmask, unsigned int numretvals) {
  if (callargs == NULL)
    return result;
  if (result == NULL || numretvals == 0) {
    Py_DECREF(callargs);
    return result;
  }
  Py_DECREF(result);
  fail(0);
}

static PyObject *
PyCFuncPtr_call(PyCFuncPtrObject *self, PyObject *inargs, PyObject *kwds) {
  PyObject *restype;
  PyObject *converters;
  PyObject *checker;
  PyObject *argtypes;
  StgDictObject *dict = PyObject_stgdict((PyObject *) self);
  PyObject *result;
  PyObject *callargs;
  PyObject *errcheck;
  void *pProc = NULL;

  int inoutmask;
  int outmask;
  unsigned int numretvals;

  assert(dict);

  restype = self->restype ? self->restype : dict->restype;
  converters = self->converters ? self->converters : dict->converters;
  checker = self->checker ? self->checker : dict->checker;
  argtypes = self->argtypes ? self->argtypes : dict->argtypes;
  errcheck = self->errcheck;

  pProc = *(void **) self->b_ptr;
  callargs = _build_callargs(self, argtypes,
      inargs, kwds,
      &outmask, &inoutmask, &numretvals);
  if (callargs == NULL)
    return NULL;

  if (converters) {
    fail(0);
  }

  result = _ctypes_callproc(pProc,
    callargs,
    dict->flags,
    converters,
    restype,
    checker);

  if (result != NULL && errcheck) {
    fail(0);
  }

  return _build_result(result, callargs,
      outmask, inoutmask, numretvals);
}

static int
PyCFuncPtr_clear(PyCFuncPtrObject *self) {
  fail(0);
}

static PyGetSetDef PyCFuncPtr_getsets[] = {
  {NULL, NULL},
};

static int
_get_name(PyObject *obj, const char **pname) {
  if (PyBytes_Check(obj)) {
    fail(0);
  }
  if (PyUnicode_Check(obj)) {
    *pname = PyUnicode_AsUTF8(obj);
    return *pname ? 1 : 0;
  }
  fail(0);
}

static int
_validate_paramflags(PyTypeObject *type, PyObject *paramflags) {
  if (paramflags == NULL) {
    return 1;
  }
  fail(0);
}

static int PyCData_MallocBuffer(CDataObject *obj, StgDictObject *dict) {
  if ((size_t) dict->size <= sizeof(obj->b_value)) {
    obj->b_ptr = (char *)&obj->b_value;
    obj->b_needsfree = 1;
  } else {
    fail(0);
  }
  obj->b_size = dict->size;
  return 0;
}

static PyObject *
GenericPyCData_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  CDataObject *obj;
  StgDictObject *dict;

  dict = PyType_stgdict((PyObject *) type);
  if (!dict) {
    fail(0);
  }
  dict->flags |= DICTFLAG_FINAL;

  obj = (CDataObject *) type->tp_alloc(type, 0);
  if (!obj)
    return NULL;

  obj->b_base = NULL;
  obj->b_index = 0;
  obj->b_objects = NULL;
  obj->b_length = dict->length;

  if (-1 == PyCData_MallocBuffer(obj, dict)) {
    fail(0);
  }
  return (PyObject *) obj;
}

static CDataObject *
PyCData_GetContainer(CDataObject *self) {
  while (self->b_base)
    self = self->b_base;
  if (self->b_objects == NULL) {
    if (self->b_length) {
      self->b_objects = PyDict_New();
      if (self->b_objects == NULL)
        return NULL;
    } else {
      fail(0);
    }
  }
  return self;
}

static PyObject *
unique_key(CDataObject *target, Py_ssize_t index) {
  char string[256];
  char *cp = string;

  cp += sprintf(cp, "%x", Py_SAFE_DOWNCAST(index, Py_ssize_t, int));
  while (target->b_base) {
    fail(0);
  }
  return PyUnicode_FromStringAndSize(string, cp - string);
}

static int
KeepRef(CDataObject *target, Py_ssize_t index, PyObject *keep) {
  int result;
  CDataObject *ob;
  PyObject *key;

  if (keep == Py_None) {
    fail(0);
  }
  ob = PyCData_GetContainer(target);
  if (ob == NULL) {
    fail(0);
  }
  if (ob->b_objects == NULL || !PyDict_CheckExact(ob->b_objects)) {
    fail(0);
  }
  key = unique_key(target, index);
  if (key == NULL) {
    fail(0);
  }
  result = PyDict_SetItem(ob->b_objects, key, keep);
  Py_DECREF(key);
  Py_DECREF(keep);
  return result;
}

static PyObject *
PyCFuncPtr_FromDll(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  const char *name;
  int (*address)(void);
  PyObject *ftuple;
  PyObject *dll;
  PyObject *obj;
  PyCFuncPtrObject *self;
  void *handle;
  PyObject *paramflags = NULL;

  if (!PyArg_ParseTuple(args, "O|O", &ftuple, &paramflags))
    return NULL;
  if (paramflags == Py_None)
    paramflags = NULL;

  ftuple = PySequence_Tuple(ftuple);
  if (!ftuple)
    return NULL;

  if (!PyArg_ParseTuple(ftuple, "O&O;illegal func_spec argument",
      _get_name, &name, &dll)) {
    fail(0);
  }

  obj = PyObject_GetAttrString(dll, "_handle");
  if (!obj) {
    fail(0);
  }
  if (!PyLong_Check(obj)) {
    fail(0);
  }
  handle = (void *) PyLong_AsVoidPtr(obj);
  Py_DECREF(obj);
  if (PyErr_Occurred()) {
    fail(0);
  }
  
  address = (PPROC) ctypes_dlsym(handle, name);
  if (!address) {
    fail(0);
  }

  if (!_validate_paramflags(type, paramflags)) {
    fail(0);
  }
  
  self = (PyCFuncPtrObject *) GenericPyCData_new(type, args, kwds);
  if (!self) {
    fail(0);
  }

  Py_XINCREF(paramflags);
  self->paramflags = paramflags;

  *(void **) self->b_ptr = address;
  Py_INCREF(dll);
  Py_DECREF(ftuple);
  if (-1 == KeepRef((CDataObject *) self, 0, dll)) {
    fail(0);
  }

  Py_INCREF(self);
  self->callable = (PyObject *) self;
  return (PyObject *) self;
}

static PyObject *
PyCFuncPtr_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  if (PyTuple_GET_SIZE(args) == 0) {
    fail(0);
  }

  if (1 <= PyTuple_GET_SIZE(args) && PyTuple_Check(PyTuple_GET_ITEM(args, 0)))
    return PyCFuncPtr_FromDll(type, args, kwds);
  fail(0);
}

PyTypeObject PyCFuncPtr_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "_ctypes.CFuncPtr",
  .tp_basicsize = sizeof(PyCFuncPtrObject),
  .tp_dealloc = (destructor) PyCFuncPtr_dealloc,
  .tp_call = (ternaryfunc) PyCFuncPtr_call,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_clear = (inquiry) PyCFuncPtr_clear,
  .tp_getset = PyCFuncPtr_getsets,
  .tp_new = PyCFuncPtr_new,
};

static int
_ctypes_add_objects(PyObject *mod) {
#define MOD_ADD(name, expr) \
  do { \
    PyObject *obj = (expr); \
    if (obj == NULL) { \
      return -1; \
    } \
    if (PyModule_AddObjectRef(mod, name, obj) < 0) { \
      Py_DECREF(obj); \
      return -1; \
    } \
    Py_DECREF(obj); \
  } while (0)

  MOD_ADD("FUNCFLAG_CDECL", PyLong_FromLong(FUNCFLAG_CDECL));
  MOD_ADD("RTLD_LOCAL", PyLong_FromLong(RTLD_LOCAL));
  return 0;
#undef MOD_ADD
}

static int
CDataType_clear(PyTypeObject *self) {
  fail(0);
}

static PyMethodDef CDataType_methods[] = {
  {NULL, NULL},
};

static PyCArgObject *
PyCFuncPtrType_paramfunc(CDataObject *self) {
  fail(0);
}

static int
make_funcptrtype_dict(StgDictObject *stgdict) {
  PyObject *ob;
  _Py_IDENTIFIER(_flags_);
  _Py_IDENTIFIER(_argtypes_);
  _Py_IDENTIFIER(_restype_);
  _Py_IDENTIFIER(_check_retval_);

  stgdict->align = _ctypes_get_fielddesc("P")->pffi_type->alignment;
  stgdict->length = 1;
  stgdict->size = sizeof(void *);
  stgdict->setfunc = NULL;
  stgdict->getfunc = NULL;
  stgdict->ffi_type_pointer = ffi_type_pointer;

  ob = _PyDict_GetItemIdWithError((PyObject *) stgdict, &PyId__flags_);
  if (!ob || !PyLong_Check(ob)) {
    fail(0);
  }
  stgdict->flags = PyLong_AsUnsignedLongMask(ob) | TYPEFLAG_ISPOINTER;

  ob = _PyDict_GetItemIdWithError((PyObject *) stgdict, &PyId__argtypes_);
  if (ob) {
    fail(0);
  } else if (PyErr_Occurred()) {
    fail(0);
  }

  ob = _PyDict_GetItemIdWithError((PyObject *) stgdict, &PyId__restype_);
  if (ob) {
    if (ob != Py_None && !PyType_stgdict(ob) && !PyCallable_Check(ob)) {
      fail(0);
    }
    Py_INCREF(ob);
    stgdict->restype = ob;
    if (_PyObject_LookupAttrId(ob, &PyId__check_retval_,
        &stgdict->checker) < 0) {
      return -1;
    }
  } else if (PyErr_Occurred()) {
    return -1;
  }

  return 0;
}

static PyObject *
PyCFuncPtrType_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  PyTypeObject *result;
  StgDictObject *stgdict;

  stgdict = (StgDictObject *) _PyObject_CallNoArg(
      (PyObject *) &PyCStgDict_Type);
  if (!stgdict)
    return NULL;

  stgdict->paramfunc = PyCFuncPtrType_paramfunc;
  fprintf(stderr, "\033[31m PyCFuncPtrType_new should set stgdict.format \033[0m\n");
  stgdict->format = NULL;

  stgdict->flags |= TYPEFLAG_ISPOINTER;

  // create the new instance (which is a class, since we are a metatype!)
  result = (PyTypeObject *) PyType_Type.tp_new(type, args, kwds);
  if (result == NULL) {
    fail(0);
  }

  if (-1 == PyDict_Update((PyObject *) stgdict, result->tp_dict)) {
    fail(0);
  }

  Py_SETREF(result->tp_dict, (PyObject *) stgdict);

  if (-1 == make_funcptrtype_dict(stgdict)) {
    fail(0);
  }
  return (PyObject *) result;
}

PyTypeObject PyCFuncPtrType_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "_ctypes.PyCFuncPtrType",
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_clear = (inquiry) CDataType_clear,
  .tp_methods = CDataType_methods,
  .tp_new = PyCFuncPtrType_new,
};

static void
PyCData_dealloc(PyObject *self) {
  fail(0);
}

static int
PyCData_clear(CDataObject *self) {
  fail(0);
}

static PyMethodDef PyCData_methods[] = {
  {NULL, NULL},
};

static PyMemberDef PyCData_members[] = {
  {NULL},
};

PyTypeObject PyCData_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "_ctypes._CData",
  .tp_basicsize = sizeof(CDataObject),
  .tp_dealloc = PyCData_dealloc,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_clear = (inquiry) PyCData_clear,
  .tp_methods = PyCData_methods,
  .tp_members = PyCData_members,
};

static int
_ctypes_mod_exec(PyObject *mod) {
  if (_ctypes_add_types(mod) < 0) {
    fail("_ctypes_add_types fail");
    return -1;
  }
  if (_ctypes_add_objects(mod) < 0) {
    fail("_ctypes_add_objects fail");
    return -1;
  }
  return 0;
}

PyMODINIT_FUNC
PyInit__ctypes(void) {
  PyObject *mod = PyModule_Create(&_ctypesmodule);
  if (!mod) {
    return NULL;
  }

  if (_ctypes_mod_exec(mod) < 0) {
    fail(0);
  }

  return mod;
}

static PyMethodDef PyCSimpleType_methods[] = {
  {NULL, NULL},
};

static const char SIMPLE_TYPE_CHARS[] = "i";

static PyCArgObject *
PyCSimpleType_paramfunc(CDataObject *self) {
  fail(0);
}

extern PyTypeObject PyCSimpleType_Type;
extern PyTypeObject Simple_Type;

static PyObject *
PyCSimpleType_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  _Py_IDENTIFIER(_type_);
  PyTypeObject *result;
  StgDictObject *stgdict;
  PyObject *proto;
  const char *proto_str;
  Py_ssize_t proto_len;
  PyMethodDef *ml;
  struct fielddesc *fmt;

  result = (PyTypeObject *) PyType_Type.tp_new(type, args, kwds);
  if (result == NULL) {
    return NULL;
  }

  if (_PyObject_LookupAttrId((PyObject *) result, &PyId__type_, &proto) < 0) {
    return NULL;
  }
  if (!proto) {
    fail("missing '_type_'");
  }

  if (PyUnicode_Check(proto)) {
    proto_str = PyUnicode_AsUTF8AndSize(proto, &proto_len);
    if (!proto_str)
      fail(0);
  } else {
    fail(0);
  }
  if (proto_len != 1) {
    fail(0);
  }

  if (!strchr(SIMPLE_TYPE_CHARS, *proto_str)) {
    fail(0);
  }
  fmt = _ctypes_get_fielddesc(proto_str);
  if (fmt == NULL) {
    fail(0);
  }

  stgdict = (StgDictObject *) _PyObject_CallNoArg(
    (PyObject *) &PyCStgDict_Type);
  if (!stgdict)
    fail(0);

  stgdict->ffi_type_pointer = *fmt->pffi_type;
  stgdict->align = fmt->pffi_type->alignment;
  stgdict->length = 0;
  stgdict->size = fmt->pffi_type->size;
  stgdict->setfunc = fmt->setfunc;
  stgdict->getfunc = fmt->getfunc;
  fprintf(stderr, "\033[31m PyCSimpleType_new should set stgdict.format \033[0m\n");
  stgdict->format = NULL;

  stgdict->paramfunc = PyCSimpleType_paramfunc; 
  stgdict->proto = proto;

  if (-1 == PyDict_Update((PyObject *) stgdict, result->tp_dict)) {
    fail(0);
  }
  Py_SETREF(result->tp_dict, (PyObject *) stgdict);
  if (result->tp_base == &Simple_Type) {
    switch (*proto_str) {
    case 'z':
    case 'Z':
    case 'P':
    case 's':
    case 'X':
    case 'O':
      fail(0);
    default:
      ml = NULL;
      break;
    }
    if (ml) {
      fail(0);
    }
  }
  if (type == &PyCSimpleType_Type && fmt->setfunc_swapped && fmt->getfunc_swapped) {
    fprintf(stderr, "\033[33m PyCSimpleType_new setup swapped type \033[0m\n");
  }
  return (PyObject *) result;
}

PyTypeObject PyCSimpleType_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "_ctypes.PyCSimpleType",
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_methods = PyCSimpleType_methods,
  .tp_new = PyCSimpleType_new,
};

static PyMethodDef Simple_methods[] = {
  {NULL, NULL},
};

static PyGetSetDef Simple_getsets[] = {
  {NULL, NULL},
};

static int
Simple_init(CDataObject *self, PyObject *args, PyObject *kw) {
  fail(0);
}

PyTypeObject Simple_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "_ctypes._SimpleCData",
  .tp_basicsize = sizeof(CDataObject),
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_clear = (inquiry) PyCData_clear,
  .tp_methods = Simple_methods,
  .tp_getset = Simple_getsets,
  .tp_init = (initproc) Simple_init,
  .tp_new = GenericPyCData_new,
};

static int _ctypes_add_types(PyObject *mod) {
#define TYPE_READY(TYPE) \
  if (PyType_Ready(TYPE) < 0) { \
    return -1; \
  }

#define TYPE_READY_BASE(TYPE_EXPR, TP_BASE) \
  do { \
    PyTypeObject *type = (TYPE_EXPR); \
    type->tp_base = (TP_BASE); \
    TYPE_READY(type); \
  } while (0)

#define MOD_ADD_TYPE(TYPE_EXPR, TP_TYPE, TP_BASE) \
  do { \
    PyTypeObject *type = (TYPE_EXPR); \
    Py_SET_TYPE(type, TP_TYPE); \
    type->tp_base = TP_BASE; \
    if (PyModule_AddType(mod, type) < 0) { \
      fail("fail to add type to mod"); \
      return -1; \
    } \
  } while (0)
 
  TYPE_READY(&PyCArg_Type);
  TYPE_READY(&PyCData_Type);
  TYPE_READY_BASE(&PyCStgDict_Type, &PyDict_Type);

  // Metaclasses
  TYPE_READY_BASE(&PyCSimpleType_Type, &PyType_Type);
  TYPE_READY_BASE(&PyCFuncPtrType_Type, &PyType_Type);

  MOD_ADD_TYPE(&Simple_Type, &PyCSimpleType_Type, &PyCData_Type);
  MOD_ADD_TYPE(&PyCFuncPtr_Type, &PyCFuncPtrType_Type, &PyCData_Type);

#undef TYPE_READY
#undef TYPE_READY_BASE
#undef MOD_ADD_TYPE

  return 0;
}

int _ctypes_simple_instance(PyObject *obj) {
  PyTypeObject *type = (PyTypeObject *) obj;

  if (PyCSimpleTypeObject_Check(type))
    return type->tp_base != &Simple_Type;
  return 0;
}
