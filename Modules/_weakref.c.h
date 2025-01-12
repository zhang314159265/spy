#pragma once

static PyMethodDef
weakref_functions[] = {
  {NULL, NULL, 0, NULL}
};

static int
weakref_exec(PyObject *module) {
  Py_INCREF(&_PyWeakref_RefType);
  if (PyModule_AddObject(module, "ref", (PyObject *) &_PyWeakref_RefType) < 0) {
    fail(0);
  }

  return 0;
}

static struct PyModuleDef_Slot weakref_slots[] = {
  {Py_mod_exec, weakref_exec},
  {0, NULL},
};

static struct PyModuleDef weakrefmodule = {
  PyModuleDef_HEAD_INIT,
  "_weakref",
  "Weak-reference support module.",
  0,
  weakref_functions,
  weakref_slots,
};

PyMODINIT_FUNC
PyInit__weakref(void) {
  return PyModuleDef_Init(&weakrefmodule);
}
