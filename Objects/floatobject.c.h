#pragma once

static int convert_to_double(PyObject **v, double *dbl) {
	PyObject *obj = *v;

	if (PyLong_Check(obj)) {
		*dbl = PyLong_AsDouble(obj);
		if (*dbl == -1.0 && PyErr_Occurred()) {
			*v = NULL;
			return -1;
		}
	} else {
		assert(false);
	}
	return 0;
}

