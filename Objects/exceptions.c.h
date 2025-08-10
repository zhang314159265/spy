#pragma once

#include "structmember.h"

static int BaseException_clear(PyBaseExceptionObject *self);
static void BaseException_dealloc(PyBaseExceptionObject *self);
static int OSError_init(PyOSErrorObject *self, PyObject *args, PyObject *kwds);
static PyObject *OSError_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

static PyObject *
BaseException_str(PyBaseExceptionObject *self) {
  switch (PyTuple_GET_SIZE(self->args)) {
  case 0:
    fail(0);
  case 1:
    return PyObject_Str(PyTuple_GET_ITEM(self->args, 0));
  default:
    fail(0);
  }
  assert(false);
}

static PyMemberDef AttributeError_members[] = {
  {NULL},
};

static PyMethodDef AttributeError_methods[] = {
  {NULL},
};

#define ComplexExtendsException(EXCBASE, EXCNAME, EXCSTORE, EXCNEW, \
    EXCMETHODS, EXCMEMBERS, EXCGETSET, \
    EXCSTR, EXCDOC) \
static PyTypeObject _PyExc_ ## EXCNAME = { \
  PyVarObject_HEAD_INIT(NULL, 0) \
  .tp_name = # EXCNAME, \
  .tp_basicsize = sizeof(Py ## EXCSTORE ## Object), \
  .tp_dealloc = (destructor) EXCSTORE ## _dealloc, \
  .tp_str = (reprfunc) EXCSTR, \
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC, \
  .tp_doc = PyDoc_STR(EXCDOC), \
  .tp_traverse = (traverseproc) EXCSTORE ## _traverse, \
  .tp_clear = (inquiry) EXCSTORE ## _clear, \
  .tp_methods = EXCMETHODS, \
  .tp_members = EXCMEMBERS, \
  .tp_getset = EXCGETSET, \
  .tp_base = &_ ## EXCBASE, \
  .tp_dictoffset = offsetof(Py ## EXCSTORE ## Object, dict), \
  .tp_init = (initproc) EXCSTORE ## _init, \
  .tp_new = EXCNEW, \
}; \
PyObject *PyExc_ ## EXCNAME = (PyObject *) &_PyExc_ ## EXCNAME

#define SimpleExtendsException(EXCBASE, EXCNAME, EXCDOC) \
static PyTypeObject _PyExc_ ## EXCNAME = { \
  PyVarObject_HEAD_INIT(NULL, 0) \
  .tp_name = # EXCNAME, \
  .tp_basicsize = sizeof(PyBaseExceptionObject), \
  .tp_dealloc = (destructor) BaseException_dealloc, \
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC, \
  .tp_doc = PyDoc_STR(EXCDOC), \
  .tp_traverse = (traverseproc) BaseException_traverse, \
  .tp_clear = (inquiry) BaseException_clear, \
  .tp_base = &_ ## EXCBASE, \
  .tp_dictoffset = offsetof(PyBaseExceptionObject, dict), \
  .tp_init = (initproc) BaseException_init, \
  .tp_new = BaseException_new, \
}; \
PyObject *PyExc_ ## EXCNAME = (PyObject *) &_PyExc_ ## EXCNAME

static int
BaseException_traverse(PyBaseExceptionObject *self, visitproc visit, void *arg) {
  assert(false);
}

static int
BaseException_clear(PyBaseExceptionObject *self) {
  Py_CLEAR(self->dict);
  Py_CLEAR(self->args);
  Py_CLEAR(self->traceback);
  Py_CLEAR(self->cause);
  Py_CLEAR(self->context);
  return 0;
}

static void BaseException_dealloc(PyBaseExceptionObject *self) {
  BaseException_clear(self);
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static int BaseException_init(PyBaseExceptionObject *self, PyObject *args, PyObject *kwds) {
  if (!_PyArg_NoKeywords(Py_TYPE(self)->tp_name, kwds))
    return -1;

  Py_INCREF(args);
  Py_XSETREF(self->args, args);

  return 0;
}

static PyObject *
BaseException_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  PyBaseExceptionObject *self;

  self = (PyBaseExceptionObject *) type->tp_alloc(type, 0);
  if (!self)
    return NULL;
  self->dict = NULL;
  self->traceback = self->cause = self->context = NULL;
  self->suppress_context = 0;

  if (args) {
    self->args = args;
    Py_INCREF(args);
    return (PyObject *) self;
  }

  self->args = PyTuple_New(0);
  if (!self->args) {
    Py_DECREF(self);
    return NULL;
  }
  return (PyObject *) self;
}

static PyObject *
BaseException_repr(PyBaseExceptionObject *self) {
  fail(0);
}

static PyTypeObject _PyExc_BaseException = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "BaseException",
  .tp_basicsize = sizeof(PyBaseExceptionObject),
  .tp_dealloc = (destructor) BaseException_dealloc,
  .tp_getattro = PyObject_GenericGetAttr,
  .tp_setattro = PyObject_GenericSetAttr,
  .tp_dictoffset = offsetof(PyBaseExceptionObject, dict),
  .tp_init = (initproc) BaseException_init,
  .tp_new = BaseException_new,
  .tp_flags = Py_TPFLAGS_BASE_EXC_SUBCLASS,
  .tp_str = (reprfunc) BaseException_str,
  .tp_repr = (reprfunc) BaseException_repr,
};

PyObject *PyExc_BaseException = (PyObject *) &_PyExc_BaseException;

SimpleExtendsException(PyExc_BaseException, Exception,
    "Common base class for all non-exit exceptions.");

SimpleExtendsException(PyExc_Exception, AssertionError,
    "Assertion failed.");

SimpleExtendsException(PyExc_Exception, RuntimeError,
    "Unspecified run-time error.");

SimpleExtendsException(PyExc_Exception, ValueError,
    "Inappropriate argument value (of correct type).");

SimpleExtendsException(PyExc_Exception, TypeError,
    "Inappropriate argument type.");

static int AttributeError_init(PyAttributeErrorObject *self, PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {"name", "obj", NULL};
  PyObject *name = NULL;
  PyObject *obj = NULL;

  if (BaseException_init((PyBaseExceptionObject *) self, args, NULL) == -1) {
    return -1;
  }

  PyObject *empty_tuple = PyTuple_New(0);
  if (!empty_tuple) {
    return -1;
  }
  if (!PyArg_ParseTupleAndKeywords(empty_tuple, kwds, "|$OO:AttributeError", kwlist, &name, &obj)) {
    Py_DECREF(empty_tuple);
    return -1;
  }
  Py_DECREF(empty_tuple);

  Py_XINCREF(name);
  Py_XSETREF(self->name, name);

  Py_XINCREF(obj);
  Py_XSETREF(self->obj, obj);

  return 0;
}

static int AttributeError_clear(PyAttributeErrorObject *self) {
  Py_CLEAR(self->obj);
  Py_CLEAR(self->name);
  return BaseException_clear((PyBaseExceptionObject *) self);
}

static void AttributeError_dealloc(PyAttributeErrorObject *self) {
  AttributeError_clear(self);
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static int AttributeError_traverse(PyAttributeErrorObject *self, visitproc visit, void *arg) {
  assert(false);
}

ComplexExtendsException(PyExc_Exception, AttributeError,
    AttributeError, 0,
    AttributeError_methods, AttributeError_members,
    0, BaseException_str, "Attribute not found.");

static PyMemberDef ImportError_members[] = {
  {NULL},
};

static PyMethodDef ImportError_methods[] = {
  {NULL},
};

static PyObject *
ImportError_str(PyImportErrorObject *self) {
  if (self->msg && PyUnicode_CheckExact(self->msg)) {
    Py_INCREF(self->msg);
    return self->msg;
  } else {
    return BaseException_str((PyBaseExceptionObject *) self);
  }
}

static int
ImportError_traverse(PyImportErrorObject *self, visitproc visit, void *arg) {
  fail(0);
}

static void
ImportError_dealloc(PyImportErrorObject *self) {
  fail(0);
}

static int
ImportError_init(PyImportErrorObject *self, PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {"name", "path", 0};
  PyObject *empty_tuple;
  PyObject *msg = NULL;
  PyObject *name = NULL;
  PyObject *path = NULL;

  if (BaseException_init((PyBaseExceptionObject *) self, args, NULL) == -1)
    return -1;
  empty_tuple = PyTuple_New(0);
  if (!empty_tuple)
    return -1;
  if (!PyArg_ParseTupleAndKeywords(empty_tuple, kwds, "|$OO:ImportError", kwlist, &name, &path)) {
    Py_DECREF(empty_tuple);
    return -1;
  }
  Py_DECREF(empty_tuple);

  Py_XINCREF(name);
  Py_XSETREF(self->name, name);

  Py_XINCREF(path);
  Py_XSETREF(self->path, path);

  if (PyTuple_GET_SIZE(args) == 1) {
    msg = PyTuple_GET_ITEM(args, 0);
    Py_INCREF(msg);
  }
  Py_XSETREF(self->msg, msg);

  return 0;
}

static int
ImportError_clear(PyImportErrorObject *self) {
  fail(0);
}

ComplexExtendsException(PyExc_Exception, ImportError,
    ImportError, 0,
    ImportError_methods, ImportError_members,
    0, ImportError_str,
    "Import can't find module, or can't find name in module.");

static int oserror_use_init(PyTypeObject *type);

static int
oserror_parse_args(PyObject **p_args,
    PyObject **myerrno, PyObject **strerror,
    PyObject **filename, PyObject **filename2
    ) {
  Py_ssize_t nargs;
  PyObject *args = *p_args;
  PyObject *_winerror = NULL;
  PyObject **winerror = &_winerror;

  nargs = PyTuple_GET_SIZE(args);

  if (nargs >= 2 && nargs <= 5) {
    if (!PyArg_UnpackTuple(args, "OSError", 2, 5,
        myerrno, strerror,
        filename, winerror, filename2))
      return -1;
  }
  return 0;
}

#if 0
static struct _Py_exc_state *
get_exc_state(void) {
  PyInterpreterState *interp = _PyInterpreterState_GET();
  return &interp->exc_state;
}
#endif

static int
oserror_init(PyOSErrorObject *self, PyObject **p_args,
    PyObject *myerrno, PyObject *strerror,
    PyObject *filename, PyObject *filename2) {
  PyObject *args = *p_args;
  Py_ssize_t nargs = PyTuple_GET_SIZE(args);

  if (filename && filename != Py_None) {
    // if (Py_IS_TYPE(self, (PyTypeObject *) PyExc_BlockingIOError) && PyNumber_Check(filename)) {
    if (PyNumber_Check(filename)) {
      fail(0);
    } else {
      Py_INCREF(filename);
      self->filename = filename;

      if (filename2 && filename2 != Py_None) {
        fail(0);
      }

      if (nargs >= 2 && nargs <= 5) {
        PyObject *subslice = PyTuple_GetSlice(args, 0, 2);
        if (!subslice)
          return -1;
        Py_DECREF(args);
        *p_args = args = subslice;
      }
    }
  }

  Py_XINCREF(myerrno);
  self->myerrno = myerrno;

  Py_XINCREF(strerror);
  self->strerror = strerror;

  Py_XSETREF(self->args, args);
  *p_args = args = NULL;

  return 0;
}

static PyObject *
OSError_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  PyOSErrorObject *self = NULL;
  PyObject *myerrno = NULL, *strerror = NULL;
  PyObject *filename = NULL, *filename2 = NULL;

  Py_INCREF(args);
  if (!oserror_use_init(type)) {
    if (!_PyArg_NoKeywords(type->tp_name, kwds)) {
      fail(0);
    }

    if (oserror_parse_args(&args, &myerrno, &strerror,
        &filename, &filename2)) {
      fail(0);
    }

    #if 0
    struct _Py_exc_state *state = get_exc_state();
    fail(0);
    #endif
    // TODO use _Py_exc_state.errormap
  }

  self = (PyOSErrorObject *) type->tp_alloc(type, 0);
  if (!self)
    fail(0);

  self->dict = NULL;
  self->traceback = self->cause = self->context = NULL;
  self->written = -1;

  if (!oserror_use_init(type)) {
    if (oserror_init(self, &args, myerrno, strerror, filename, filename2)) 
      fail(0);
  } else {
    fail(0);
  }

  Py_XDECREF(args);
  return (PyObject *) self;
}

static int
OSError_init(PyOSErrorObject *self, PyObject *args, PyObject *kwds) {
  if (!oserror_use_init(Py_TYPE(self)))
    return 0;
  fail(0);
}

static int OSError_clear(PyOSErrorObject *self);

static void
OSError_dealloc(PyOSErrorObject *self) {
  OSError_clear(self);
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static int OSError_clear(PyOSErrorObject *self) {
  Py_CLEAR(self->myerrno);
  Py_CLEAR(self->strerror);
  Py_CLEAR(self->filename);
  Py_CLEAR(self->filename2);
  return BaseException_clear((PyBaseExceptionObject *) self);
}

static int
OSError_traverse(PyOSErrorObject *self, visitproc visit, void *arg) {
  fail(0);
}

static PyMemberDef OSError_members[] = {
  {NULL},
};

static PyMethodDef OSError_methods[] = {
  {NULL},
};

static PyGetSetDef OSError_getset[] = {
  {NULL},
};

static PyObject *
OSError_str(PyOSErrorObject *self) {
#define OR_NONE(x) ((x)?(x) : Py_None)
  if (self->filename) {
    if (self->filename2) {
      fail(0);
    } else {
      return PyUnicode_FromFormat("[Errno %S] %S: %R",
          OR_NONE(self->myerrno),
          OR_NONE(self->strerror),
          self->filename);
    }
  }
  fail(0);
}

ComplexExtendsException(PyExc_Exception, OSError,
    OSError, OSError_new, OSError_methods, OSError_members, OSError_getset,
        OSError_str, "Base class for I/O related errors.");

#define MiddlingExtendsException(EXCBASE, EXCNAME, EXCSTORE, EXCDOC) \
static PyTypeObject _PyExc_ ## EXCNAME = { \
  PyVarObject_HEAD_INIT(NULL, 0) \
  .tp_name = # EXCNAME, \
  .tp_basicsize = sizeof(Py ## EXCSTORE ## Object), \
  .tp_dealloc = (destructor) EXCSTORE ## _dealloc, \
  .tp_flags = Py_TPFLAGS_BASETYPE, \
  .tp_clear = (inquiry) EXCSTORE ## _clear, \
  .tp_base = &_ ## EXCBASE, \
  .tp_dictoffset = offsetof(Py ## EXCSTORE ## Object, dict), \
  .tp_init = (initproc) EXCSTORE ## _init, \
}; \
PyObject *PyExc_ ## EXCNAME = (PyObject *) &_PyExc_ ## EXCNAME

// ModuleNotFoundError extends ImportError
MiddlingExtendsException(PyExc_ImportError, ModuleNotFoundError, ImportError, "Module not found.");

MiddlingExtendsException(PyExc_OSError, FileNotFoundError, OSError, "File not found.");

MiddlingExtendsException(PyExc_OSError, PermissionError, OSError,
    "Not enough permissions.");

MiddlingExtendsException(PyExc_OSError, NotADirectoryError, OSError,
    "Operation only works on directoris.");

static PyObject *
KeyError_str(PyBaseExceptionObject *self) {
  fail(0);
}

SimpleExtendsException(PyExc_Exception, LookupError,
  "Base class for lookup errors.");

ComplexExtendsException(PyExc_LookupError, KeyError, BaseException,
    0, 0, 0, 0, KeyError_str, "Mapping key not found.");

PyStatus _PyExc_Init(PyInterpreterState *interp) {
#define PRE_INIT(TYPE) \
  if (!(_PyExc_ ## TYPE.tp_flags & Py_TPFLAGS_READY)) { \
    if (PyType_Ready(&_PyExc_ ## TYPE) < 0) { \
      assert(false); \
    } \
    Py_INCREF(PyExc_ ## TYPE); \
  }
  PRE_INIT(AttributeError);
  PRE_INIT(RuntimeError);
  PRE_INIT(ImportError);
  PRE_INIT(LookupError);
  PRE_INIT(KeyError);
  PRE_INIT(ModuleNotFoundError);
  PRE_INIT(OSError);
  PRE_INIT(FileNotFoundError);
  PRE_INIT(PermissionError);
  PRE_INIT(NotADirectoryError);
  PRE_INIT(TypeError);
  PRE_INIT(ValueError);
  PRE_INIT(AssertionError);
  return _PyStatus_OK();
}

PyStatus
_PyBuiltins_AddExceptions(PyObject *bltinmod) {
#define POST_INIT(TYPE) \
    if (PyDict_SetItemString(bdict, # TYPE, PyExc_ ## TYPE)) { \
      assert(false); \
    }

  PyObject *bdict;

  bdict = PyModule_GetDict(bltinmod);
  if (bdict == NULL) {
    assert(false);
  }

  POST_INIT(RuntimeError);
  POST_INIT(AttributeError);
  POST_INIT(ImportError);
  POST_INIT(LookupError);
  POST_INIT(KeyError);
  POST_INIT(ModuleNotFoundError);
  POST_INIT(OSError);
  POST_INIT(FileNotFoundError);
  POST_INIT(PermissionError);
  POST_INIT(NotADirectoryError);
  POST_INIT(TypeError);
  POST_INIT(ValueError);
  POST_INIT(AssertionError);
  return _PyStatus_OK();
#undef POST_INIT
#undef INIT_ALIAS
}

static int BaseException_set_tb(PyBaseExceptionObject *self, PyObject *tb, void *ignored) {
  if (tb == NULL) {
    fail(0);
  } else if (!(tb == Py_None || PyTraceBack_Check(tb))) {
    fail(0);
  }

  Py_INCREF(tb);
  Py_XSETREF(self->traceback, tb);
  return 0;
}


static int
oserror_use_init(PyTypeObject *type) {
  if (type->tp_init != (initproc) OSError_init &&
      type->tp_new == (newfunc) OSError_new) {
    assert((PyObject *) type != PyExc_OSError);
    return 1;
  }
  return 0;
}
