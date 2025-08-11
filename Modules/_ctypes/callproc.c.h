#pragma once

#include "ceval.h"

#define max(a, b) ((a) > (b) ? (a) : (b))

static PyObject *py_dl_open(PyObject *self, PyObject *args) {
  PyObject *name, *name2;
  const char *name_str;
  void *handle;
  int mode = RTLD_NOW | RTLD_LOCAL;

  if (!PyArg_ParseTuple(args, "O|i:dlopen", &name, &mode))
    return NULL;

  mode |= RTLD_NOW;
  if (name != Py_None) {
    if (PyUnicode_FSConverter(name, &name2) == 0)
      return NULL;
    name_str = PyBytes_AS_STRING(name2);
  } else {
    fail(0);
  }
  handle = ctypes_dlopen(name_str, mode);
  Py_XDECREF(name2);
  if (!handle) {
    fail(0);
  }
  return PyLong_FromVoidPtr(handle);
}

PyMethodDef _ctypes_module_methods[] = {
  {"dlopen", py_dl_open, METH_VARARGS, ""}, 
  {NULL, NULL},
};

#define CTYPES_MAX_ARGCOUNT 1024

union result {
  int i;
  void *p;
};

struct argument {
  ffi_type *ffi_type;
  PyObject *keep;
  union result value;
};

ffi_type *_ctypes_get_ffi_type(PyObject *obj) {
  StgDictObject *dict;
  if (obj == NULL) {
    return &ffi_type_sint;
  }
  dict = PyType_stgdict(obj);
  if (dict == NULL) {
    fail(0);
  }
  return &dict->ffi_type_pointer;
}

static int ConvParam(PyObject *obj, Py_ssize_t index, struct argument *pa) {
  StgDictObject *dict;
  pa->keep = NULL;

  dict = PyObject_stgdict(obj);
  if (dict) {
    fail(0);
  }

  if (PyCArg_CheckExact(obj)) {
    fail(0);
  }

  if (obj == Py_None) {
    fail(0);
  }

  if (PyLong_Check(obj)) {
    pa->ffi_type = &ffi_type_sint;
    pa->value.i = (long) PyLong_AsUnsignedLong(obj);
    if (pa->value.i == -1 && PyErr_Occurred()) {
      fail(0);
    }
    return 0;
  }

  if (PyBytes_Check(obj)) {
    fail(0);
  }

  if (PyUnicode_Check(obj)) {
    fail(0);
  }
  fail(0);
}

static PyObject *GetResult(PyObject *restype, void *result, PyObject *checker) {
  StgDictObject *dict;
  PyObject *retval, *v;

  if (restype == NULL) {
    fail(0);
  }
  if (restype == Py_None) {
    Py_RETURN_NONE;
  }

  dict = PyType_stgdict(restype);
  if (dict == NULL)
    fail(0);

  if (dict->getfunc && !_ctypes_simple_instance(restype)) {
    retval = dict->getfunc(result, dict->size);

    if (dict->getfunc == _ctypes_get_fielddesc("O")->getfunc) {
      Py_DECREF(retval);
    }
  } else {
    fail(0);
  }

  if (!checker || !retval)
    return retval;

  fail(0);
}

static int _call_function_pointer(int flags,
    PPROC pProc,
    void **avalues,
    ffi_type **atypes,
    ffi_type *restype,
    void *resmem,
    int argcount,
    int argtypecount) {

  PyObject *error_object = NULL;
  int *space;
  ffi_cif cif;
  int cc;

  if (restype == NULL) {
    fail(0);
  }

  cc = FFI_DEFAULT_ABI;

  bool is_variadic = (argtypecount != 0 && argcount > argtypecount);
  (void) is_variadic;

  if (is_variadic) {
    fail(0);
  } else {
    if (FFI_OK != ffi_prep_cif(&cif,
        cc,
        argcount,
        restype,
        atypes)) {
      fail(0);
    }
  }

  if (flags & (FUNCFLAG_USE_ERRNO | FUNCFLAG_USE_LASTERROR)) {
    fail(0);
  }
  if ((flags & FUNCFLAG_PYTHONAPI) == 0)
    Py_UNBLOCK_THREADS
  if (flags & FUNCFLAG_USE_ERRNO) {
    int temp = space[0];
    space[0] = errno;
    errno = temp;
  }

  ffi_call(&cif, (void *) pProc, resmem, avalues);

  if (flags & FUNCFLAG_USE_ERRNO) {
    fail(0);
  }
  if ((flags & FUNCFLAG_PYTHONAPI) == 0)
    Py_BLOCK_THREADS

  Py_XDECREF(error_object);
  if ((flags & FUNCFLAG_PYTHONAPI) && PyErr_Occurred())
    return -1;
  return 0;
}

PyObject *_ctypes_callproc(PPROC pProc,
    PyObject *argtuple,
    int flags,
    PyObject *argtypes,
    PyObject *restype,
    PyObject *checker) {
  Py_ssize_t i, n, argcount, argtype_count;
  void *resbuf;
  struct argument *args, *pa;
  ffi_type **atypes;
  ffi_type *rtype;
  void **avalues;
  PyObject *retval = NULL;

  n = argcount = PyTuple_GET_SIZE(argtuple);

  if (argcount > CTYPES_MAX_ARGCOUNT) {
    fail(0);
  } 

  args = (struct argument *) alloca(sizeof(struct argument) * argcount);
  if (!args) {
    fail(0);
  }
  memset(args, 0, sizeof(struct argument) * argcount);
  argtype_count = argtypes ? PyTuple_GET_SIZE(argtypes) : 0;
  pa = &args[0];

  for (i = 0; i < n; ++i, ++pa) {
    PyObject *arg;
    int err;

    arg = PyTuple_GET_ITEM(argtuple, i);

    if (argtypes && argtype_count > i) {
      fail(0);
    } else {
      err = ConvParam(arg, i + 1, pa);
      if (-1 == err) {
        fail(0);
      }
    }
  }

  rtype = _ctypes_get_ffi_type(restype);
  resbuf = alloca(max(rtype->size, sizeof(ffi_arg)));

  avalues = (void **) alloca(sizeof(void *) * argcount);
  atypes = (ffi_type **) alloca(sizeof(ffi_type *) * argcount);
  if (!resbuf || !avalues || !atypes) {
    fail(0);
  }
  for (i = 0; i < argcount; ++i) {
    atypes[i] = args[i].ffi_type;

    if (atypes[i]->type == FFI_TYPE_STRUCT)
      fail(0);
    else
      avalues[i] = (void *)&args[i].value;
  }
  if (-1 == _call_function_pointer(flags, pProc, avalues, atypes,
      rtype, resbuf,
      Py_SAFE_DOWNCAST(argcount, Py_ssize_t, int),
      Py_SAFE_DOWNCAST(argtype_count, Py_ssize_t, int)))
    fail(0);
  retval = GetResult(restype, resbuf, checker);
cleanup:
  for (i = 0; i < argcount; ++i)
    Py_XDECREF(args[i].keep);
  return retval;
}

static void
PyCArg_dealloc(PyCArgObject *self) {
  fail(0);
}

static PyMemberDef PyCArgType_members[] = {
  {NULL},
};

PyTypeObject PyCArg_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "CArgObject",
  .tp_basicsize = sizeof(PyCArgObject),
  .tp_dealloc = (destructor) PyCArg_dealloc,
  .tp_flags = Py_TPFLAGS_DEFAULT,
  .tp_members = PyCArgType_members,
};
