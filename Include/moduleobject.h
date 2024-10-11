#pragma once

#include "methodobject.h"

typedef struct PyModuleDef_Base {
	PyObject_HEAD
	Py_ssize_t m_index;
} PyModuleDef_Base;

struct PyModuleDef_Slot;

typedef struct PyModuleDef {
	PyModuleDef_Base m_base;
	const char *m_name;
	const char *m_doc;
	Py_ssize_t m_size;
	PyMethodDef *m_methods;
	struct PyModuleDef_Slot* m_slots;
} PyModuleDef;

// defined in cpy/Include/internal/pycore_moduleobject.h
typedef struct {
	PyObject_HEAD
	PyObject *md_dict;
	struct PyModuleDef *md_def;
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
