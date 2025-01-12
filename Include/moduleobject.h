#pragma once

#include "methodobject.h"

PyObject *
PyModule_GetNameObject(PyObject *m);

typedef struct PyModuleDef_Base {
	PyObject_HEAD
  PyObject *(*m_init)(void);
	Py_ssize_t m_index;
} PyModuleDef_Base;

struct PyModuleDef_Slot;

typedef struct PyModuleDef_Slot {
  int slot;
  void *value;
} PyModuleDef_Slot;

#define Py_mod_create 1
#define Py_mod_exec 2

#define _Py_mod_LAST_SLOT 2

typedef struct PyModuleDef {
	PyModuleDef_Base m_base;
	const char *m_name;
	const char *m_doc;
	Py_ssize_t m_size;
	PyMethodDef *m_methods;
	struct PyModuleDef_Slot* m_slots;
  traverseproc m_traverse;
  inquiry m_clear;
  freefunc m_free;
} PyModuleDef;

// defined in cpy/Include/internal/pycore_moduleobject.h
typedef struct {
	PyObject_HEAD
	PyObject *md_dict;
	struct PyModuleDef *md_def;
  void *md_state;
  PyObject *md_weaklist;
	// for logging purposes after md_dict is cleared
	PyObject *md_name;
} PyModuleObject;

#define PyModuleDef_HEAD_INIT { \
	PyObject_HEAD_INIT(NULL) \
	.m_index = 0, \
}

extern PyTypeObject PyModule_Type;
#define PyModule_Check(op) PyObject_TypeCheck(op, &PyModule_Type)

static inline PyObject *_PyModule_GetDict(PyObject *mod) {
	assert(PyModule_Check(mod));
	PyObject *dict = ((PyModuleObject *) mod)->md_dict;
	assert(dict != NULL);
	return dict;
}

PyObject *PyModuleDef_Init(struct PyModuleDef *def);

PyTypeObject PyModuleDef_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "moduledef",
  .tp_basicsize = sizeof(struct PyModuleDef),
};

static inline PyModuleDef *_PyModule_GetDef(PyObject *mod) {
  assert(PyModule_Check(mod));
  return ((PyModuleObject *) mod)->md_def;
}

PyModuleDef *
PyModule_GetDef(PyObject *m) {
  if (!PyModule_Check(m)) {
    fail(0);
  }
  return _PyModule_GetDef(m);
}

static inline void *_PyModule_GetState(PyObject *mod) {
  assert(PyModule_Check(mod));
  return ((PyModuleObject *) mod)->md_state;
}

void *
PyModule_GetState(PyObject *m) {
  if (!PyModule_Check(m)) {
    fail(0);
  }
  return _PyModule_GetState(m);
}

const char *
PyModule_GetName(PyObject *m) {
  PyObject *name = PyModule_GetNameObject(m);
  if (name == NULL)
    return NULL;
  Py_DECREF(name);
  return PyUnicode_AsUTF8(name);
}

int
PyModule_ExecDef(PyObject *module, PyModuleDef *def) {
  PyModuleDef_Slot *cur_slot;
  const char *name;
  int ret;

  name = PyModule_GetName(module);
  if (name == NULL) {
    return -1;
  }

  if (def->m_size >= 0) {
    PyModuleObject *md = (PyModuleObject *) module;
    if (md->md_state == NULL) {
      md->md_state = PyMem_Malloc(def->m_size);
      if (!md->md_state) {
        fail(0);
      }
      memset(md->md_state, 0, def->m_size);
    }
  }

  if (def->m_slots == NULL) {
    return 0;
  }

  for (cur_slot = def->m_slots; cur_slot && cur_slot->slot; cur_slot++) {
    switch (cur_slot->slot) {
    case Py_mod_create:
      break;
    case Py_mod_exec:
      ret = ((int (*)(PyObject *))cur_slot->value)(module);
      if (ret != 0) {
        fail(0);
      }
      if (PyErr_Occurred()) {
        fail(0);
      }
      break;
    default:
      fail(0);
    }
  }
  return 0;
}

PyObject *_PyObject_GetAttrId(PyObject *v, _Py_Identifier *name);

int _PyModuleSpec_IsInitializing(PyObject *spec) {
  if (spec != NULL) {
    _Py_IDENTIFIER(_initializing);
    PyObject *value = _PyObject_GetAttrId(spec, &PyId__initializing);
    if (value != NULL) {
      int initializing = PyObject_IsTrue(value);
      Py_DECREF(value);
      if (initializing >= 0) {
        return initializing;
      }
    }
  }
  PyErr_Clear();
  return 0;
}
