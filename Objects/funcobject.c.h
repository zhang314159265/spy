#pragma once

static PyGetSetDef func_getsetlist[] = {
  {"__name__", (getter) func_get_name, (setter) func_set_name},
  {NULL}
};

PyTypeObject PyFunction_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "function",
	.tp_basicsize = sizeof(PyFunctionObject),
  .tp_dealloc = (destructor) func_dealloc,
	.tp_flags = Py_TPFLAGS_HAVE_VECTORCALL | Py_TPFLAGS_METHOD_DESCRIPTOR,
	.tp_call = PyVectorcall_Call,
	.tp_vectorcall_offset = offsetof(PyFunctionObject, vectorcall),
  .tp_getset = func_getsetlist,
};
