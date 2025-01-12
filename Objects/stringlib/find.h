#define FORMAT_BUFFER_SIZE 50

int
STRINGLIB(parse_args_finds)(const char *function_name, PyObject *args,
    PyObject **subobj,
    Py_ssize_t *start, Py_ssize_t *end) {

  PyObject *tmp_subobj;
  Py_ssize_t tmp_start = 0;
  Py_ssize_t tmp_end = PY_SSIZE_T_MAX;
  PyObject *obj_start = Py_None, *obj_end = Py_None;
  char format[FORMAT_BUFFER_SIZE] = "O|OO:";
  size_t len = strlen(format);

  strncpy(format + len, function_name, FORMAT_BUFFER_SIZE - len - 1);
  format[FORMAT_BUFFER_SIZE - 1] = '\0';

  if (!PyArg_ParseTuple(args, format, &tmp_subobj, &obj_start, &obj_end))
    return 0;

  if (obj_start != Py_None)
    if (!_PyEval_SliceIndex(obj_start, &tmp_start))
      return 0;

  if (obj_end != Py_None)
    if (!_PyEval_SliceIndex(obj_end, &tmp_end))
      return 0;

  *start = tmp_start;
  *end = tmp_end;
  *subobj = tmp_subobj;
  return 1;
}
