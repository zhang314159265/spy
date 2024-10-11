#pragma once

#include "moduleobject.h"
#include "modsupport.h"

PyDoc_STRVAR(print_doc,
"print...");

PyObject *_PySys_GetObjectId(_Py_Identifier *key);

static PyObject *
builtin_print(PyObject *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames) {
	#if 1 // my dummy implementation
	for (int i = 0; i < nargs; i++) {
		if (i > 0) {
			printf(" ");
		}
		assert(PyUnicode_Check(args[i]));
		printf("%s", PyUnicode_1BYTE_DATA(args[i]));
	}
	printf("\n");
	Py_RETURN_NONE;
	#endif

	#if 0
	_Py_IDENTIFIER(stdout);

	PyObject *file = NULL;
	if (kwnames != NULL) {
		assert(false);
	}

	if (file == NULL || file == Py_None) {
		file = _PySys_GetObjectId(&PyId_stdout);
		if (file == NULL) {
			assert(false);
		}

		// sys.stdout may be None when FILE* stdout isn't connected
		if (file == Py_None)
			Py_RETURN_NONE;
	}
	assert(false);
	#endif
}

static PyMethodDef builtin_methods[] = {
	{"print", (PyCFunction)(void(*)(void))builtin_print, METH_FASTCALL | METH_KEYWORDS, print_doc },
	{NULL, NULL},
};

PyDoc_STRVAR(builtin_doc,
"Built-in functions, exceptions, and other objects.\n\
\n\
Noteworthy: None is the `nil' objectt; Ellipsis represents `...' in slices.");

static struct PyModuleDef builtinsmodule = {
	PyModuleDef_HEAD_INIT,
	.m_name = "builtins",
	.m_doc = builtin_doc,	
	.m_size = -1,
	.m_methods = builtin_methods,
};

PyObject *
_PyBuiltin_Init(PyInterpreterState *interp)
{
	PyObject *mod;

	mod = _PyModule_CreateInitialized(&builtinsmodule, PYTHON_API_VERSION);
	if (mod == NULL)
		return NULL;
	return mod;
}
