typedef struct {
  PyObject_VAR_HEAD
  PyObject **ob_item;

  // capacity
  Py_ssize_t allocated;
} PyListObject;
