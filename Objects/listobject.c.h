#pragma once

static PyMethodDef list_methods[] = {
	LIST_APPEND_METHODDEF
	{NULL, NULL},
};

// defined in cpy/Objects/listobject.c
PyTypeObject PyList_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "list",
  .tp_basicsize = sizeof(PyListObject),
  .tp_flags = Py_TPFLAGS_LIST_SUBCLASS,
	.tp_dealloc = (destructor) list_dealloc,
	.tp_free = PyObject_GC_Del,
	.tp_as_sequence = &list_as_sequence,
	.tp_repr = (reprfunc) list_repr,
	.tp_methods = list_methods,
	.tp_getattro = PyObject_GenericGetAttr,
};
