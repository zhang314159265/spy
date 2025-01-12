PyDoc_STRVAR(builtin_abs__doc__, "");

#define BUILTIN_ABS_METHODDEF \
	{"abs", (PyCFunction) builtin_abs, METH_O, builtin_abs__doc__},

#define BUILTIN_HASATTR_METHODDEF \
  {"hasattr", (PyCFunction)(void (*)(void))builtin_hasattr, METH_FASTCALL, ""},

#define BUILTIN_SETATTR_METHODDEF \
  {"setattr", (PyCFunction)(void(*)(void)) builtin_setattr, METH_FASTCALL, ""},

#define BUILTIN_ISINSTANCE_METHODDEF \
  {"isinstance", (PyCFunction)(void(*)(void))builtin_isinstance, METH_FASTCALL, ""},

static PyObject *
builtin_isinstance_impl(PyObject *module, PyObject *obj, PyObject *class_or_tuple);

static PyObject *
builtin_isinstance(PyObject *module, PyObject *const *args, Py_ssize_t nargs) {
  PyObject *return_value = NULL;
  PyObject *obj;
  PyObject *class_or_tuple;

  if (!_PyArg_CheckPositional("isinstance", nargs, 2, 2)) {
    goto exit;
  }
  obj = args[0];
  class_or_tuple = args[1];
  return_value = builtin_isinstance_impl(module, obj, class_or_tuple);

exit:
  return return_value;
}
