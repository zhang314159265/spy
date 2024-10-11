#pragma once

#include "descrobject.h"

// Generic type check
// defined in cpy/Objects/typeobject.c
int PyType_IsSubtype(PyTypeObject *a, PyTypeObject *b) {
  PyObject *mro;

  mro = a->tp_mro;
	if (mro != NULL) {
		Py_ssize_t i, n;
		assert(PyTuple_Check(mro));
		n = PyTuple_GET_SIZE(mro);
		for (i = 0; i < n; i++) {
			if (PyTuple_GET_ITEM(mro, i) == (PyObject *) b)
				return 1;
		}
		return 0;
	}
	else {
		printf("type name is a %s b %s\n", a->tp_name, b->tp_name);
  	assert(false);
	}
}

static PyObject *
type_call(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	assert(false);
}

PyTypeObject PyType_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "type",
  .tp_basicsize = sizeof(PyHeapTypeObject),
  .tp_flags = Py_TPFLAGS_TYPE_SUBCLASS | Py_TPFLAGS_HAVE_VECTORCALL,
	.tp_call = (ternaryfunc) type_call,
	.tp_vectorcall_offset = offsetof(PyTypeObject, tp_vectorcall),
};

static int type_add_method(PyTypeObject *type, PyMethodDef *meth) {
	PyObject *descr;
	int isdescr = 1;

	if (meth->ml_flags & METH_CLASS) {
		assert(false);
	} else if (meth->ml_flags & METH_STATIC) {
		assert(false);
	} else {
		descr = PyDescr_NewMethod(type, meth);
	}
	if (descr == NULL) {
		return -1;
	}

	PyObject *name;
	if (isdescr) {
		name = PyDescr_NAME(descr);
	} else {
		assert(false);
	}

	int err;
	if (!(meth->ml_flags & METH_COEXIST)) {
		err = PyDict_SetDefault(type->tp_dict, name, descr) == NULL;
	} else {
		assert(false);
	}
	if (!isdescr) {
		Py_DECREF(name);
	}
	Py_DECREF(descr);
	if (err) {
		return -1;
	}
	return 0;
}

static int type_add_methods(PyTypeObject *type) {
	PyMethodDef *meth = type->tp_methods;
	if (meth == NULL) {
		return 0;
	}
	for (; meth->ml_name != NULL; meth++) {
		if (type_add_method(type, meth) < 0) {
			return -1;
		}
	}
	return 0;
}


