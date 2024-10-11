#pragma once

PyTypeObject PyModule_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "module",
	.tp_basicsize = sizeof(PyModuleObject),
	.tp_setattro = PyObject_GenericSetAttr,
	.tp_dictoffset = offsetof(PyModuleObject, md_dict),
};

PyObject *
PyModule_GetDict(PyObject *m) {
	if (!PyModule_Check(m)) {
		assert(false);
	}
	return _PyModule_GetDict(m);
}
