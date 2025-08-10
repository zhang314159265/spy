// Should be auto generate as cpy/build/Modules/config.c

extern PyObject *PyInit__imp(void);
extern PyObject *PyInit__thread(void);
extern PyObject *_PyWarnings_Init(void);
extern PyObject *PyInit__weakref(void);
extern PyObject *PyInit__io(void);
extern PyObject *PyMarshal_Init(void);
extern PyObject *PyInit_posix(void);
extern PyObject *PyInit_math(void);
extern PyObject *PyInit__ctypes(void);

struct _inittab _PyImport_Inittab[] = {
  {"_imp", PyInit__imp},
  {"_thread", PyInit__thread},
  {"_warnings", _PyWarnings_Init},
  {"_weakref", PyInit__weakref},
  {"_io", PyInit__io},
  {"marshal", PyMarshal_Init},
  {"posix", PyInit_posix},
  {"sys", NULL},
  {"math", PyInit_math}, // this is from cpy/PC/config.c

  // XXX cpy load _ctypes as extension module, but I load it as a builtin module
  {"_ctypes", PyInit__ctypes},
  {0, 0},
};
