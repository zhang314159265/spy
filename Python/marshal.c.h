#pragma once

static int
marshal_module_exec(PyObject *mod) {
  return 0;
}

static PyMethodDef marshal_methods[] = {
  {NULL, NULL},
};

static PyModuleDef_Slot marshalmodule_slots[] = {
  {Py_mod_exec, marshal_module_exec},
  {0, NULL},
};

static struct PyModuleDef marshalmodule = {
  PyModuleDef_HEAD_INIT,
  .m_name = "marshal",
  .m_doc = "",
  .m_methods = marshal_methods,
  .m_slots = marshalmodule_slots,
};

PyMODINIT_FUNC
PyMarshal_Init(void) {
  return PyModuleDef_Init(&marshalmodule);
}
