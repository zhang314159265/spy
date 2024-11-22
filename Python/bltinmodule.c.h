#pragma once

#include "moduleobject.h"
#include "modsupport.h"
#include "rangeobject.h"
#include "cellobject.h"

#include "Python/clinic/bltinmodule.c.h"

_Py_IDENTIFIER(__prepare__);

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
		if (PyUnicode_Check(args[i])) {
			printf("%s", PyUnicode_1BYTE_DATA(args[i]));
		} else if (PyLong_Check(args[i])) {
			printf("%ld", PyLong_AsLong(args[i]));
		} else {
			printf("%s", PyUnicode_1BYTE_DATA(PyObject_Str(args[i])));
		}
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

static PyObject *builtin_abs(PyObject *module, PyObject *x);
static PyObject *builtin___build_class__(PyObject *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames);

PyDoc_STRVAR(build_class_doc, "");

static PyMethodDef builtin_methods[] = {
  {"__build_class__", (PyCFunction)(void(*)(void))builtin___build_class__, METH_FASTCALL | METH_KEYWORDS, build_class_doc},
	{"print", (PyCFunction)(void(*)(void))builtin_print, METH_FASTCALL | METH_KEYWORDS, print_doc },
	BUILTIN_ABS_METHODDEF
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
	PyObject *mod, *dict;

	mod = _PyModule_CreateInitialized(&builtinsmodule, PYTHON_API_VERSION);
	if (mod == NULL)
		return NULL;
	dict = PyModule_GetDict(mod);

#define ADD_TO_ALL(OBJECT) (void)0

#define SETBUILTIN(NAME, OBJECT) \
	if (PyDict_SetItemString(dict, NAME, (PyObject *) OBJECT) < 0) \
		return NULL; \
	ADD_TO_ALL(OBJECT)

	SETBUILTIN("range", &PyRange_Type);
  SETBUILTIN("tuple", &PyTuple_Type);

	return mod;
#undef ADD_TO_ALL
#undef SETBUILTIN
}

static PyObject *
builtin_abs(PyObject *module, PyObject *x) {
	return PyNumber_Absolute(x);
}

static PyObject *
update_bases(PyObject *bases, PyObject *const *args, Py_ssize_t nargs) {
  Py_ssize_t i;
  PyObject *new_bases = NULL;

  for (i = 0; i < nargs; i++) {
    assert(false);
  }
  if (!new_bases) {
    return bases;
  }
  assert(false);
}

PyTypeObject *_PyType_CalculateMetaclass(PyTypeObject *, PyObject *);
PyObject *PyObject_VectorcallDict(PyObject *callable, PyObject *const *args, size_t nargsf, PyObject *kwargs);

static PyObject *builtin___build_class__(PyObject *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames) {
  PyObject *func, *name, *winner, *prep;
  PyObject *cls = NULL, *cell = NULL, *ns = NULL, *meta = NULL, *orig_bases = NULL;
  PyObject *mkw = NULL, *bases = NULL;
  int isclass = 0;

  if (nargs < 2) {
    assert(false);
  }
  func = args[0];
  if (!PyFunction_Check(func)) {
    assert(false);
  }
  name = args[1];
  if (!PyUnicode_Check(name)) {
    assert(false);
  }
  printf("__build_class__ for %s, nargs %ld, kwnames %p\n", (char *) PyUnicode_DATA(name), nargs, kwnames);
  orig_bases = _PyTuple_FromArray(args + 2, nargs - 2);
  if (orig_bases == NULL)
    return NULL;

  bases = update_bases(orig_bases, args + 2, nargs - 2);
  if (bases == NULL) {
    assert(false);
  }

  if (kwnames == NULL) {
    meta = NULL;
    mkw = NULL;
  } else {
    assert(false);
  }
  if (meta == NULL) {
    // If there are no bases, use type
    if (PyTuple_GET_SIZE(bases) == 0) {
      meta = (PyObject *) (&PyType_Type);
    }
    // else get the type of the first base
    else {
      assert(false);
    }
    Py_INCREF(meta);
    isclass = 1;
  }

  if (isclass) {
    winner = (PyObject *) _PyType_CalculateMetaclass((PyTypeObject *) meta, bases);
    if (winner == NULL) {
      assert(false);
    }
    if (winner != meta) {
      Py_DECREF(meta);
      meta = winner;
      Py_INCREF(meta);
    }
  }
  if (_PyObject_LookupAttrId(meta, &PyId___prepare__, &prep) < 0) {
    ns = NULL;
  } else if (prep == NULL) {
    ns = PyDict_New();
  } else {
    PyObject *pargs[2] = {name, bases};
    ns = PyObject_VectorcallDict(prep, pargs, 2, mkw);
    Py_DECREF(prep);
  }
  if (ns == NULL) {
    assert(false);
  }
  if (!PyMapping_Check(ns)) {
    assert(false);
  }
  PyFrameConstructor *f = PyFunction_AS_FRAME_CONSTRUCTOR(func);
  PyThreadState *tstate = PyThreadState_GET();
  cell = _PyEval_Vector(tstate, f, ns, NULL, 0, NULL);
  if (cell != NULL) {
    if (bases != orig_bases) {
      assert(false);
    }
    PyObject *margs[3] = {name, bases, ns};
    cls = PyObject_VectorcallDict(meta, margs, 3, mkw);
    if (cls != NULL && PyType_Check(cls) && PyCell_Check(cell)) {
      assert(false);
    }
  }
 error:
  Py_XDECREF(cell);
  Py_XDECREF(ns);
  Py_XDECREF(meta);
  Py_XDECREF(mkw);
  if (bases != orig_bases) {
    Py_DECREF(orig_bases);
  }
  Py_DECREF(bases);
  return cls;
}
