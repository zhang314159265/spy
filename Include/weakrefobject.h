#pragma once

#define GET_WEAKREFS_LISTPTR(o) \
  ((PyWeakReference **) _PyObject_GET_WEAKREFS_LISTPTR(o))

#define PyWeakref_GET_OBJECT(ref) \
  (Py_REFCNT(((PyWeakReference *)(ref))->wr_object) > 0 \
   ? ((PyWeakReference *) (ref))->wr_object \
   : Py_None)

extern PyTypeObject _PyWeakref_RefType;
#define PyWeakref_CheckRef(op) PyObject_TypeCheck(op, &_PyWeakref_RefType)

// TODO check proxy
#define PyWeakref_Check(op) \
  (PyWeakref_CheckRef(op))

typedef struct _PyWeakReference PyWeakReference;

struct _PyWeakReference {
  PyObject_HEAD

  PyObject *wr_object;
  PyObject *wr_callback;
  Py_hash_t hash;

  PyWeakReference *wr_prev;
  PyWeakReference *wr_next;
};

static void
get_basic_refs(PyWeakReference *head,
    PyWeakReference **refp, PyWeakReference **proxyp) {
  *refp = NULL;
  *proxyp = NULL;

  if (head != NULL && head->wr_callback == NULL) {
    assert(false);
  }
}

static int
parse_weakref_init_args(const char *funcname, PyObject *args, PyObject *kwargs, PyObject **obp, PyObject **callbackp);

int
_PyArg_NoKeywords(const char *funcname, PyObject *kwargs);

static int
weakref___init__(PyObject *self, PyObject *args, PyObject *kwargs) {
  PyObject *tmp;

  if (!_PyArg_NoKeywords("ref", kwargs))
    return -1;

  if (parse_weakref_init_args("__init__", args, kwargs, &tmp, &tmp))
    return 0;
  else 
    return -1;
}

int PyArg_UnpackTuple(PyObject *, const char *, Py_ssize_t, Py_ssize_t, ...);

static int
parse_weakref_init_args(const char *funcname, PyObject *args, PyObject *kwargs, PyObject **obp, PyObject **callbackp) {
  return PyArg_UnpackTuple(args, funcname, 1, 2, obp, callbackp);
}

static void init_weakref(PyWeakReference *self, PyObject *ob, PyObject *callback);
static void
insert_head(PyWeakReference *newref, PyWeakReference **list);

static PyObject *
weakref___new__(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
  PyWeakReference *self = NULL;
  PyObject *ob, *callback = NULL;

  if (parse_weakref_init_args("__new__", args, kwargs, &ob, &callback)) {
    PyWeakReference *ref, *proxy;
    PyWeakReference **list;

    if (!PyType_SUPPORTS_WEAKREFS(Py_TYPE(ob))) {
      fail(0);
    }
    if (callback == Py_None)
      callback = NULL;
    list = GET_WEAKREFS_LISTPTR(ob);
    get_basic_refs(*list, &ref, &proxy);
    if (callback == NULL && type == &_PyWeakref_RefType) {
      if (ref != NULL) {
        fail(0);
      }
    }
    // We have to create a new reference
    self = (PyWeakReference *) (type->tp_alloc(type, 0));
    if (self != NULL) {
      init_weakref(self, ob, callback);
      if (callback == NULL && type == &_PyWeakref_RefType) {
        // insert_head(self, list);
        fail(0);
      } else {
        PyWeakReference *prev;

        get_basic_refs(*list, &ref, &proxy);
        prev = (proxy == NULL) ? ref : proxy;
        if (prev == NULL) {
          insert_head(self, list);
        } else {
          fail(0);
        }
      }
    }
  }
  return (PyObject *) self;
}

static PyObject *
weakref_call(PyWeakReference *self, PyObject *args, PyObject *kw) {
  assert(false);
}

static void
clear_weakref(PyWeakReference *self) {
  PyObject *callback = self->wr_callback;

  if (self->wr_object != Py_None) {
    PyWeakReference **list = GET_WEAKREFS_LISTPTR(self->wr_object);

    if (*list == self)
      *list = self->wr_next;
    self->wr_object = Py_None;
    if (self->wr_prev != NULL)
      self->wr_prev->wr_next = self->wr_next;
    if (self->wr_next != NULL)
      self->wr_next->wr_prev = self->wr_prev;
    self->wr_prev = self->wr_next = NULL;
  }
  if (callback != NULL) {
    assert(false);
  }
}

static void
weakref_dealloc(PyObject *self) {
  clear_weakref((PyWeakReference *) self);
  Py_TYPE(self)->tp_free(self);
}

static PyMethodDef weakref_methods[] = {
  {NULL}
};

static PyMemberDef weakref_members[] = {
  {NULL},
};

PyTypeObject
_PyWeakref_RefType = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "weakref.ReferenceType",
  .tp_basicsize = sizeof(PyWeakReference),
  .tp_dealloc = weakref_dealloc,
  .tp_call = (ternaryfunc) weakref_call,
  .tp_flags = Py_TPFLAGS_BASETYPE,
  .tp_methods = weakref_methods,
  .tp_members = weakref_members,
  .tp_init = weakref___init__,
  .tp_alloc = PyType_GenericAlloc,
  .tp_new = weakref___new__,
  .tp_free = PyObject_GC_Del,
};

static void
init_weakref(PyWeakReference *self, PyObject *ob, PyObject *callback) {
  self->hash = -1;
  self->wr_object = ob;
  self->wr_prev = NULL;
  self->wr_next = NULL;
  Py_XINCREF(callback);
  self->wr_callback = callback;
}

static PyWeakReference *
new_weakref(PyObject *ob, PyObject *callback) {
  PyWeakReference *result;

  result = PyObject_GC_New(PyWeakReference, &_PyWeakref_RefType);
  if (result) {
    init_weakref(result, ob, callback);
  }
  return result;
}

static void
insert_head(PyWeakReference *newref, PyWeakReference **list) {
  PyWeakReference *next = *list;

  newref->wr_prev = NULL;
  newref->wr_next = next;
  if (next != NULL)
    next->wr_prev = newref;
  *list = newref;
}

PyObject *
PyWeakref_NewRef(PyObject *ob, PyObject *callback) {
  PyWeakReference *result = NULL;
  PyWeakReference **list;
  PyWeakReference *ref, *proxy;

  if (!PyType_SUPPORTS_WEAKREFS(Py_TYPE(ob))) {
    fprintf(stderr, "type %s does not support weak ref\n", Py_TYPE(ob)->tp_name);
    assert(false);
  }
  list = GET_WEAKREFS_LISTPTR(ob);
  get_basic_refs(*list, &ref, &proxy);
  if (callback == Py_None)
    callback = NULL;
  if (callback == NULL)
    result = ref;
  if (result != NULL)
    Py_INCREF(result);
  else {
    result = new_weakref(ob, callback);
    if (result != NULL) {
      get_basic_refs(*list, &ref, &proxy);
      if (callback == NULL) {
        if (ref == NULL)
          insert_head(result, list);
        else {
          assert(false);
        }
      } else {
        assert(false);
      }
    }
  }
  return (PyObject *) result;
}

PyObject *
PyWeakref_GetObject(PyObject *ref) {
  if (ref == NULL || !PyWeakref_Check(ref)) {
    assert(false);
  }
  return PyWeakref_GET_OBJECT(ref);
}
