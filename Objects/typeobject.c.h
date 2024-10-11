#pragma once

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


