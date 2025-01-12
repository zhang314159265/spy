#pragma once

int _PyUnicode_FormatAdvancedWriter(_PyUnicodeWriter *writer, PyObject *obj, PyObject *format_spec, Py_ssize_t start, Py_ssize_t end) {
  assert(PyUnicode_Check(obj));

  if (start == end) {
    if (PyUnicode_CheckExact(obj))
      return _PyUnicodeWriter_WriteStr(writer, obj);
    else
      fail(0);
  }
  fail(0);
}

int _PyLong_FormatAdvancedWriter(_PyUnicodeWriter *writer,
		PyObject *obj,
		PyObject *format_spec,
		Py_ssize_t start, Py_ssize_t end) {
	if (start == end) {
		if (PyLong_CheckExact(obj)) {
			return _PyLong_FormatWriter(writer, obj, 10, 0);
		} else {
			assert(false);
		}
	}
	assert(false);
}

static int
format_obj(PyObject *obj, _PyUnicodeWriter *writer) {
	PyObject *str;
	int err;

	str = PyObject_Str(obj);
	if (str == NULL)
		return -1;

	err = _PyUnicodeWriter_WriteStr(writer, str);
	Py_DECREF(str);
	return err;
}

int _PyFloat_FormatAdvancedWriter(_PyUnicodeWriter *writer,
		PyObject *obj, PyObject *format_spec, Py_ssize_t start, Py_ssize_t end) {
	if (start == end)
		return format_obj(obj, writer);
	assert(false);
}
