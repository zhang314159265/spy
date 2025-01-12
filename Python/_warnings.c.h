#pragma once

#define MODULE_NAME "_warnings"

static PyMethodDef warnings_functions[] = {
  {NULL, NULL},
};

static int warnings_module_exec(PyObject *module) {
  // fail(0); // TODO follow cpy
  return 0;
}

static PyModuleDef_Slot warnings_slots[] = {
  {Py_mod_exec, warnings_module_exec},
  {0, NULL}, 
};

static struct PyModuleDef warnings_module = {
  PyModuleDef_HEAD_INIT,
  .m_name = MODULE_NAME,
  .m_doc = "",
  .m_size = 0,
  .m_methods = warnings_functions,
  .m_slots = warnings_slots,
};

PyMODINIT_FUNC
_PyWarnings_Init(void) {
  return PyModuleDef_Init(&warnings_module);
}
