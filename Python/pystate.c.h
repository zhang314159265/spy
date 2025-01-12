#pragma once

int
_PyState_AddModule(PyThreadState *tstate, PyObject *module, struct PyModuleDef *def) {
  if (!def) {
    fail(0);
  }
  if (def->m_slots) {
    fail(0);
  }
  PyInterpreterState *interp = tstate->interp;
  if (!interp->modules_by_index) {
    interp->modules_by_index = PyList_New(0);
    if (!interp->modules_by_index) {
      return -1;
    }
  }

  while (PyList_GET_SIZE(interp->modules_by_index) <= def->m_base.m_index) {
    if (PyList_Append(interp->modules_by_index, Py_None) < 0) {
      return -1;
    }
  }
  Py_INCREF(module);
  return PyList_SetItem(interp->modules_by_index,
      def->m_base.m_index, module);
}
