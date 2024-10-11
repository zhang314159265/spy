#pragma once

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
