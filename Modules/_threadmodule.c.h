#pragma once

#include "typeslots.h"
#include "pytime.h"

typedef struct {
  PyTypeObject *lock_type;
} thread_module_state;

typedef struct {
  PyObject_HEAD
  PyThread_type_lock lock_lock;
  PyObject *in_weakreflist;
  char locked;
} lockobject;

static lockobject *newlockobject(PyObject *module);

static PyObject *
thread_PyThread_allocate_lock(PyObject *module, PyObject *ignored) {
  return (PyObject*) newlockobject(module);
}

static PyObject *
thread_get_ident(PyObject *self, PyObject *ignored) {
  unsigned long ident = PyThread_get_thread_ident();
  if (ident == PYTHREAD_INVALID_THREAD_ID) {
    fail(0);
  }
  return PyLong_FromUnsignedLong(ident);
}

static PyMethodDef thread_methods[] = {
  {"allocate_lock", thread_PyThread_allocate_lock, METH_NOARGS, ""},
  {"get_ident", thread_get_ident, METH_NOARGS, ""},
  {NULL, NULL}
};

static int thread_module_traverse(PyObject *module, visitproc visit, void *arg) {
  fail(0);
}

static int thread_module_clear(PyObject *module) {
  fail(0);
}

static void thread_module_free(void *module) {
  fail(0);
}

static int thread_module_exec(PyObject *module);

static PyModuleDef_Slot thread_module_slots[] = {
  {Py_mod_exec, thread_module_exec},
  {0, NULL},
};

static struct PyModuleDef thread_module = {
  PyModuleDef_HEAD_INIT,
  .m_name = "_thread",
  .m_doc = "",
  .m_size = sizeof(thread_module_state),
  .m_methods = thread_methods,
  // .m_traverse = thread_module_traverse,
  // .m_clear = thread_module_clear,
  // .m_free = thread_module_free,
  .m_slots = thread_module_slots,
};

PyMODINIT_FUNC
PyInit__thread(void) {
  return PyModuleDef_Init(&thread_module);
}

static inline thread_module_state*
get_thread_state(PyObject *module) {
  void *state = _PyModule_GetState(module);
  assert(state != NULL);
  return (thread_module_state *) state;
}

static lockobject *
newlockobject(PyObject *module) {
  thread_module_state *state = get_thread_state(module);

  PyTypeObject *type = state->lock_type;
  lockobject *self = (lockobject *) type->tp_alloc(type, 0);
  if (self == NULL) {
    return NULL;
  }

  self->lock_lock = PyThread_allocate_lock();
  self->locked = 0;
  self->in_weakreflist = NULL;

  if (self->lock_lock == NULL) {
    fail(0);
  }
  return self;
}

static int
lock_acquire_parse_args(PyObject *args, PyObject *kwds,
    _PyTime_t *timeout) {
  fail(0);
}

static PyObject *
lock_PyThread_acquire_lock(lockobject *self, PyObject *args, PyObject *kwds) {
  // TODO follow cpy. Right now this is no op
  #if 0
  _PyTime_t timeout;
  if (lock_acquire_parse_args(args, kwds, &timeout) < 0)
    return NULL;
  fail(0);
  #endif
  return PyBool_FromLong(1);
}

static PyObject *
lock_PyThread_release_lock(lockobject *self, PyObject *ignored) {
  // TODO follow cpy. Right now this is no op
  #if 0
  fail(0);
  #endif
  Py_RETURN_NONE;
}

static PyMethodDef lock_methods[] = {
  {"__enter__", (PyCFunction)(void(*)(void)) lock_PyThread_acquire_lock, METH_VARARGS | METH_KEYWORDS, ""},
  {"__exit__", (PyCFunction) lock_PyThread_release_lock,
    METH_VARARGS, ""},
  {NULL, NULL},
};

static PyType_Slot lock_type_slots[] = {
  {Py_tp_methods, lock_methods},
  {0, 0},
};

static PyType_Spec lock_type_spec = {
  .name = "_thread.lock",
  .basicsize = sizeof(lockobject),
  .flags = Py_TPFLAGS_HAVE_GC,
  .slots = lock_type_slots,
};

static int
thread_module_exec(PyObject *module) {
  thread_module_state *state = get_thread_state(module);
  PyObject *d = PyModule_GetDict(module);

  PyThread_init_thread();

  // Lock
  state->lock_type = (PyTypeObject *) PyType_FromSpec(&lock_type_spec);
  if (state->lock_type == NULL) {
    return -1;
  }
  if (PyDict_SetItemString(d, "LockType", (PyObject *) state->lock_type) < 0) {
    return -1;
  }

  // RLock TODO

  // Local dummy TODO

  // Local TODO

  // Add module attributes TODO
  // error
  // ExceptionHookArgsType

  // TIMEOUT_MAX

  return 0;
}


