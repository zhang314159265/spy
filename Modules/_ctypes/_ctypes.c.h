
#define ctypes_dlopen dlopen
#include <dlfcn.h>
#include "callproc.c.h"

static struct PyModuleDef _ctypesmodule = {
  PyModuleDef_HEAD_INIT,
  .m_name = "_ctypes",
  .m_doc = "",
  .m_size = -1,
  .m_methods = _ctypes_module_methods,
};

static int
_ctypes_add_objects(PyObject *mod) {
#define MOD_ADD(name, expr) \
  do { \
    PyObject *obj = (expr); \
    if (obj == NULL) { \
      return -1; \
    } \
    if (PyModule_AddObjectRef(mod, name, obj) < 0) { \
      Py_DECREF(obj); \
      return -1; \
    } \
    Py_DECREF(obj); \
  } while (0)

  MOD_ADD("RTLD_LOCAL", PyLong_FromLong(RTLD_LOCAL));
  return 0;
#undef MOD_ADD
}

static int
_ctypes_mod_exec(PyObject *mod) {
  if (_ctypes_add_objects(mod) < 0) {
    return -1;
  }
  return 0;
}

PyMODINIT_FUNC
PyInit__ctypes(void) {
  PyObject *mod = PyModule_Create(&_ctypesmodule);
  if (!mod) {
    return NULL;
  }

  if (_ctypes_mod_exec(mod) < 0) {
    fail(0);
  }

  return mod;
}
