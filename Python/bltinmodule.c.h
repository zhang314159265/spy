#pragma once

#include "moduleobject.h"
#include "modsupport.h"
#include "rangeobject.h"
#include "cellobject.h"
#include "compile.h"

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
static PyObject *builtin___import__(PyObject *self, PyObject *args, PyObject *kwds);
static PyObject *builtin_getattr(PyObject *self, PyObject *const *args, Py_ssize_t nargs);
static PyObject *builtin_hasattr(PyObject *module, PyObject *const *args, Py_ssize_t nargs);
static PyObject *builtin_setattr(PyObject *module, PyObject *const *args, Py_ssize_t nargs);

PyDoc_STRVAR(build_class_doc, "");
PyDoc_STRVAR(import_doc, "");

#define BUILTIN_EXEC_METHODDEF \
  {"exec", (PyCFunction)(void(*)(void)) builtin_exec, METH_FASTCALL, ""},

#define BUILTIN_COMPILE_METHODDEF \
  {"compile", (PyCFunction)(void(*)(void)) builtin_compile, METH_FASTCALL | METH_KEYWORDS, ""},

static PyObject *
builtin_compile_impl(PyObject *module, PyObject *source, PyObject *filename,
    const char *mode, int flags, int dont_inherit,
    int optimize, int feature_version) {
  PyObject *source_copy;
  const char *str;
  int compile_mode = -1;
  int is_ast;
  int start[] = {Py_file_input, Py_eval_input, Py_single_input, Py_func_type_input};
  PyObject *result;

  PyCompilerFlags cf = _PyCompilerFlags_INIT;
  cf.cf_flags = flags | PyCF_SOURCE_IS_UTF8;
  if (feature_version >= 0 && (flags & PyCF_ONLY_AST)) {
    cf.cf_feature_version = feature_version;
  }

  #if 0
  if (flags & ~(PyCF_MASK | PyCF_MASK_OBSOLETE | PyCF_COMPILE_MASK)) {
    fail(0);
  }
  #endif

  if (optimize < -1 || optimize > 2) {
    fail(0);
  }

  if (!dont_inherit) {
    fail(0);
  }

  if (strcmp(mode, "exec") == 0) {
    compile_mode = 0;
  } else if (strcmp(mode, "eval") == 0) {
    compile_mode = 1;
  } else {
    fail(0);
  }

  is_ast = PyAST_Check(source);
  if (is_ast == -1)
    fail(0);
  if (is_ast) {
    fail(0);
  }

  str = _Py_SourceAsString(source, "compile", "string, bytes or AST", &cf, &source_copy);
  if (str == NULL)
    fail(0);

  result = Py_CompileStringObject(str, filename, start[compile_mode], &cf, optimize);
  Py_XDECREF(source_copy);
  goto finally;

error:
  result = NULL;

finally:
  Py_DECREF(filename);
  return result;
}

static PyObject *
builtin_compile(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames) {
  PyObject *return_value = NULL;
  static const char *const _keywords[] = {"source", "filename", "mode", "flags", "dont_inherit", "optimize", "_feature_version", NULL};
  static _PyArg_Parser _parser = {NULL, _keywords, "compile", 0};
  PyObject *argsbuf[7];
  Py_ssize_t noptargs = nargs + (kwnames ? PyTuple_GET_SIZE(kwnames) : 0) - 3;
  PyObject *source;
  PyObject *filename;
  const char *mode;
  int flags;
  int dont_inherit = 0;
  int optimize = -1;
  int feature_version = -1;

  #if 0
  for (int i = 0; i < nargs + PyTuple_GET_SIZE(kwnames); ++i) {
    printf("arg %d: ", i);
    void dump_value(PyObject *);
    dump_value(args[i]);
  }
  #endif

  args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser, 3, 6, 0, argsbuf);
  if (!args) {
    fail(0);
  }
  source = args[0];
  if(!PyUnicode_FSDecoder(args[1], &filename)) {
    fail(0);
  }
  if (!PyUnicode_Check(args[2])) {
    fail(0);
  }
  Py_ssize_t mode_length;
  mode = PyUnicode_AsUTF8AndSize(args[2], &mode_length);
  if (mode == NULL) {
    fail(0);
  }
  if (strlen(mode) != (size_t) mode_length) {
    fail(0);
  }
  if (!noptargs) {
    goto skip_optional_pos;
  }
  if (args[3]) {
    fail(0);
  }
  if (args[4]) {
    dont_inherit = _PyLong_AsInt(args[4]);
    if (dont_inherit == -1 && PyErr_Occurred()) {
      fail(0);
    }
    if (!--noptargs) {
      goto skip_optional_pos;
    }
  }
  if (args[5]) {
    optimize = _PyLong_AsInt(args[5]);
    if (optimize == -1 && PyErr_Occurred()) {
      fail(0);
    }
    if (!--noptargs) {
      goto skip_optional_pos;
    }
  }
skip_optional_pos:
  if (!noptargs) {
    goto skip_optional_kwonly;
  }
  feature_version = _PyLong_AsInt(args[6]);
  if (feature_version == -1 && PyErr_Occurred()) {
    fail(0);
  }
skip_optional_kwonly:
  return_value = builtin_compile_impl(module, source, filename, mode, flags, dont_inherit, optimize, feature_version);

exit:
  return return_value;
}

static PyObject *
builtin_exec_impl(PyObject *module, PyObject *source, PyObject *globals,
    PyObject *locals) {
  PyObject *v;

  if (globals == Py_None) {
    fail(0);
  }
  else if (locals == Py_None) {
    locals = globals;
  }

  if (!PyDict_Check(globals)) {
    fail(0);
  }
  if (!PyMapping_Check(locals)) {
    fail(0);
  }
  int r = _PyDict_ContainsId(globals, &PyId___builtins__);
  if (r == 0) {
    r = _PyDict_SetItemId(globals, &PyId___builtins__,
        PyEval_GetBuiltins());
  }
  if (r < 0) {
    return NULL;
  }
  if (PyCode_Check(source)) {
    if (PyCode_GetNumFree((PyCodeObject *) source) > 0) {
      fail(0);
    }
    v = PyEval_EvalCode(source, globals, locals);
  } else {
    fail(0);
  }
  if (v == NULL)
    return NULL;
  Py_DECREF(v);
  Py_RETURN_NONE;
}

static PyObject *
builtin_exec(PyObject *module, PyObject *const *args, Py_ssize_t nargs) {
  PyObject *return_value = NULL;
  PyObject *source;
  PyObject *globals = Py_None;
  PyObject *locals = Py_None;

  if (!_PyArg_CheckPositional("exec", nargs, 1, 3)) {
    goto exit;
  }

  source = args[0];
  if (nargs < 2) {
    goto skip_optional;
  }
  globals = args[1];
  if (nargs < 3) {
    goto skip_optional;
  }
  locals = args[2];
skip_optional:
  return_value = builtin_exec_impl(module, source, globals, locals);

exit:
  return return_value;
}

#define BUILTIN_ALL_METHODDEF \
  {"all", (PyCFunction) builtin_all, METH_O, ""},

static PyObject *
builtin_all(PyObject *module, PyObject *iterable) {
  PyObject *it, *item;
  PyObject *(*iternext)(PyObject *);
  int cmp;

  it = PyObject_GetIter(iterable);
  if (it == NULL)
    return NULL;
  iternext = *Py_TYPE(it)->tp_iternext;

  for (;;) {
    item = iternext(it);
    if (item == NULL)
      break;
    cmp = PyObject_IsTrue(item);
    Py_DECREF(item);
    if (cmp < 0) {
      Py_DECREF(it);
      return NULL;
    }
    if (cmp == 0) {
      Py_DECREF(it);
      Py_RETURN_FALSE;
    }
  }
  Py_DECREF(it);
  if (PyErr_Occurred()) {
    fail(0);
  }
  Py_RETURN_TRUE;
}

#define BUILTIN_LEN_METHODDEF \
  {"len", (PyCFunction) builtin_len, METH_O, ""},

static PyObject *
builtin_len(PyObject *module, PyObject *obj) {
  Py_ssize_t res;

  res = PyObject_Size(obj);
  if (res < 0) {
    fail(0);
  }
  return PyLong_FromSsize_t(res);
}

static PyObject *
min_max(PyObject *args, PyObject *kwds, int op) {
  PyObject *v, *it, *item, *val, *maxitem, *maxval, *keyfunc = NULL;
  PyObject *emptytuple, *defaultval = NULL;;
  static char *kwlist[] = {"key", "default", NULL};
  const char *name = op == Py_LT ? "min" : "max";
  const int positional = PyTuple_Size(args) > 1;
  int ret;

  if (positional) {
    fail(0);
  } else if (!PyArg_UnpackTuple(args, name, 1, 1, &v)) {
    fail(0);
  }

  emptytuple = PyTuple_New(0);
  if (emptytuple == NULL)
    return NULL;

  ret = PyArg_ParseTupleAndKeywords(emptytuple, kwds,
    (op == Py_LT) ? "|$OO:min" : "|$OO:max",
    kwlist, &keyfunc, &defaultval);
  Py_DECREF(emptytuple);
  if (!ret)
    return NULL;

  if (positional && defaultval != NULL) {
    fail(0);
  }

  it = PyObject_GetIter(v);
  if (it == NULL) {
    return NULL;
  }
  if (keyfunc == Py_None) {
    keyfunc = NULL;
  }

  maxitem = NULL;
  maxval = NULL;
  while ((item = PyIter_Next(it))) {
    if (keyfunc != NULL) {
      fail(0);
    } else {
      val = item;
      Py_INCREF(val);
    }

    if (maxval == NULL) {
      maxitem = item;
      maxval = val;
    } else {
      fail(0);
    }
  }

  if (PyErr_Occurred())
    fail(0);
  if (maxval == NULL) {
    fail(0);
  } else {
    Py_DECREF(maxval);
  }
  Py_DECREF(it);
  return maxitem;
}

static PyObject *
builtin_max(PyObject *self, PyObject *args, PyObject *kwds) {
  return min_max(args, kwds, Py_GT);
}

static PyMethodDef builtin_methods[] = {
  {"__build_class__", (PyCFunction)(void(*)(void))builtin___build_class__, METH_FASTCALL | METH_KEYWORDS, build_class_doc},
	{"print", (PyCFunction)(void(*)(void))builtin_print, METH_FASTCALL | METH_KEYWORDS, print_doc },
  {"__import__", (PyCFunction)(void(*)(void))builtin___import__, METH_VARARGS | METH_KEYWORDS, import_doc},
  {"getattr", (PyCFunction)(void(*)(void))builtin_getattr, METH_FASTCALL, ""},
  {"max", (PyCFunction)(void(*)(void))builtin_max, METH_VARARGS | METH_KEYWORDS, ""},
	BUILTIN_ABS_METHODDEF
  BUILTIN_HASATTR_METHODDEF
  BUILTIN_SETATTR_METHODDEF
  BUILTIN_ISINSTANCE_METHODDEF
  BUILTIN_EXEC_METHODDEF
  BUILTIN_COMPILE_METHODDEF
  BUILTIN_ALL_METHODDEF
  BUILTIN_LEN_METHODDEF
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

extern PyTypeObject PyProperty_Type;
extern PyTypeObject PyStaticMethod_Type, PyClassMethod_Type;

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
  SETBUILTIN("property", &PyProperty_Type);
  SETBUILTIN("staticmethod", &PyStaticMethod_Type);
  SETBUILTIN("classmethod", &PyClassMethod_Type);
  SETBUILTIN("type", &PyType_Type);
  SETBUILTIN("object", &PyBaseObject_Type);
  SETBUILTIN("int", &PyLong_Type);
  SETBUILTIN("dict", &PyDict_Type);
  SETBUILTIN("list", &PyList_Type);
  SETBUILTIN("str", &PyUnicode_Type);
  SETBUILTIN("bytes", &PyBytes_Type);
  SETBUILTIN("set", &PySet_Type);

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
  PyObject *base, *new_bases = NULL;
  assert(PyTuple_Check(bases));

  for (i = 0; i < nargs; i++) {
    base = args[i];
    if (PyType_Check(base)) {
      if (new_bases) {
        if (PyList_Append(new_bases, base) < 0) {
          assert(false);
        }
      }
      continue;
    }
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
      PyObject *base0 = PyTuple_GET_ITEM(bases, 0);
      meta = (PyObject *) Py_TYPE(base0);
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
      PyObject *cell_cls = PyCell_GET(cell);
      if (cell_cls != cls) {
        fail(0);
      }
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

static PyObject *builtin___import__(PyObject *self, PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {"name", "globals", "locals", "fromlist",
      "level", 0};
  PyObject *name, *globals = NULL, *locals = NULL, *fromlist = NULL;
  int level = 0;

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "U|OOOi:__import__",
    kwlist, &name, &globals, &locals, &fromlist, &level))
    return NULL;
  return PyImport_ImportModuleLevelObject(name, globals, locals,
      fromlist, level);
}

static PyObject *builtin_getattr(PyObject *self, PyObject *const *args, Py_ssize_t nargs) {
  PyObject *v, *name, *result;

  if (!_PyArg_CheckPositional("getattr", nargs, 2, 3)) {
    return NULL;
  }

  v = args[0];
  name = args[1];
  if (!PyUnicode_Check(name)) {
    assert(false);
  }
  if (nargs > 2) {
    if (_PyObject_LookupAttr(v, name, &result) == 0) {
      PyObject *dflt = args[2];
      Py_INCREF(dflt);
      return dflt;
    }
  } else {
    result = PyObject_GetAttr(v, name);
  }
  return result;
}

static PyObject *
builtin_hasattr_impl(PyObject *module, PyObject *obj, PyObject *name) {
  PyObject *v;
  if (!PyUnicode_Check(name)) {
    assert(false);
  }
  if (_PyObject_LookupAttr(obj, name, &v) < 0) {
    return NULL;
  }
  if (v == NULL) {
    Py_RETURN_FALSE;
  }
  Py_DECREF(v);
  Py_RETURN_TRUE;
}

static PyObject *builtin_hasattr(PyObject *module, PyObject *const *args, Py_ssize_t nargs) {
  PyObject *return_value = NULL;
  PyObject *obj;
  PyObject *name;

  if (!_PyArg_CheckPositional("hasattr", nargs, 2, 2)) {
    assert(false);
  }
  obj = args[0];
  name = args[1];
  return_value = builtin_hasattr_impl(module, obj, name);

exit:
  return return_value;
}

static PyObject *
builtin_setattr_impl(PyObject *module, PyObject *obj, PyObject *name,
    PyObject *value) {
  if (PyObject_SetAttr(obj, name, value) != 0) {
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject *builtin_setattr(PyObject *module, PyObject *const *args, Py_ssize_t nargs) {
  PyObject *return_value = NULL;
  PyObject *obj;
  PyObject *name;
  PyObject *value;

  if (!_PyArg_CheckPositional("setattr", nargs, 3, 3)) {
    goto exit;
  }
  obj = args[0];
  name = args[1];
  value = args[2];
  return_value = builtin_setattr_impl(module, obj, name, value);
exit:
  return return_value;
}

static PyObject *
builtin_isinstance_impl(PyObject *module, PyObject *obj, PyObject *class_or_tuple) {
  int retval;

  retval = PyObject_IsInstance(obj, class_or_tuple);
  if (retval < 0)
    return NULL;
  return PyBool_FromLong(retval);
}
