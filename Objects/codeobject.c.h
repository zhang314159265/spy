#pragma once

#include "boolobject.h"

PyObject * _PyCode_ConstantKey(PyObject *op) {
	PyObject *key;

	if (op == Py_None
      || PyLong_CheckExact(op)
      || PyUnicode_CheckExact(op) || PyCode_Check(op)) {
		Py_INCREF(op);
		key = op;
  } else if (PyBool_Check(op) || PyBytes_CheckExact(op)) {
    // Make booleans different from integers 0 and 1.
		key = PyTuple_Pack(2, Py_TYPE(op), op);
  } else if (PyFloat_CheckExact(op)) {
    double d = PyFloat_AS_DOUBLE(op);
    if (d == 0.0 && copysign(1.0, d) < 0.0)
      key = PyTuple_Pack(3, Py_TYPE(op), op, Py_None);
    else
      key = PyTuple_Pack(2, Py_TYPE(op), op);
	} else if (PyBytes_CheckExact(op)) {
		key = PyTuple_Pack(2, Py_TYPE(op), op);
	} else if (PyTuple_CheckExact(op)) {
		Py_ssize_t i, len;
		PyObject *tuple;

		len = PyTuple_GET_SIZE(op);
		tuple = PyTuple_New(len);
		if (tuple == NULL)
			return NULL;

		for (i = 0; i < len; i++) {
			PyObject *item, *item_key;

			item = PyTuple_GET_ITEM(op, i);
			item_key = _PyCode_ConstantKey(item);
			if (item_key == NULL) {
				Py_DECREF(tuple);
				return NULL;
			}

			PyTuple_SET_ITEM(tuple, i, item_key);
		}

		key = PyTuple_Pack(2, tuple, op);
		Py_DECREF(tuple);
	} else {
		printf("_PyCode_ConstantKey got object of type %s\n", Py_TYPE(op)->tp_name);
		assert(false);
	}
	return key;
}


