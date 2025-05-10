#pragma once

static int
math_exec(PyObject *module) {
  return 0;
}

static PyObject *
math_1_to_whatever(PyObject *arg, double (*func)(double),
    PyObject *(*from_double_func)(double),
    int can_overflow) {
  double x, r;
  x = PyFloat_AsDouble(arg);
  if (x == -1.0 && PyErr_Occurred())
    return NULL;
  errno = 0;
  r = (*func)(x);
  // TODO handle NAN & INF
  return (*from_double_func)(r);
}

static PyObject *
math_1(PyObject *arg, double (*func)(double), int can_overflow) {
  return math_1_to_whatever(arg, func, PyFloat_FromDouble, can_overflow);
}

#define FUNC1(funcname, func, can_overflow, docstring) \
  static PyObject *math_ ## funcname(PyObject *self, PyObject *args) { \
    return math_1(args, func, can_overflow); \
  }

FUNC1(sin, sin, 0, "")

static PyMethodDef math_methods[] = {
  {"sin", math_sin, METH_O, ""},
  {NULL, NULL},
};



static PyModuleDef_Slot math_slots[] = {
  {Py_mod_exec, math_exec},
  {0, NULL},
};

static struct PyModuleDef mathmodule = {
  PyModuleDef_HEAD_INIT,
  .m_name = "math",
  .m_doc = "",
  .m_size = 0,
  .m_methods = math_methods,
  .m_slots = math_slots,
};

PyMODINIT_FUNC
PyInit_math(void) {
  return PyModuleDef_Init(&mathmodule);
}
