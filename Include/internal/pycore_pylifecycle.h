#pragma once

#include "longintrepr.h"
#include "typeobject.h"

// defined in cpy/Objects/longobject.c
int _PyLong_Init(PyInterpreterState *interp) {
	for (Py_ssize_t i = 0; i < NSMALLNEGINTS + NSMALLPOSINTS; ++i) {
		sdigit ival = (sdigit) i - NSMALLNEGINTS;
		int size = (ival < 0) ? -1 : ((ival == 0) ? 0 : 1);

		PyLongObject *v = _PyLong_New(1);
		if (!v) {
			return -1;
		}

		Py_SET_SIZE(v, size);
		v->ob_digit[0] = (digit) abs(ival);

		interp->small_ints[i] = v;
	}
	return 0;
}

PyStatus _PyTypes_InitSlotDefs(void);

extern PyTypeObject PySuper_Type;

// defined in cpy/Objects/object.c
PyStatus _PyTypes_Init() {
  PyStatus status = _PyTypes_InitSlotDefs();
  if (_PyStatus_EXCEPTION(status)) {
    return status;
  }

#define INIT_TYPE(TYPE) \
	do { \
		if (PyType_Ready(&(TYPE)) < 0) { \
			assert(false); \
		} \
	} while (0)

	// Base types
	INIT_TYPE(PyBaseObject_Type);
	INIT_TYPE(PyType_Type);
	assert(PyBaseObject_Type.tp_base == NULL);
	assert(PyType_Type.tp_base == &PyBaseObject_Type);

	INIT_TYPE(PyLong_Type);
	INIT_TYPE(_PyNone_Type);
	INIT_TYPE(PyBytes_Type);
	INIT_TYPE(PyUnicode_Type);
	INIT_TYPE(PyCode_Type);
	INIT_TYPE(PyBool_Type);
	INIT_TYPE(PyFloat_Type);
  INIT_TYPE(PyTuple_Type);
  INIT_TYPE(PyDict_Type);
  INIT_TYPE(PySuper_Type);

	return _PyStatus_OK();

#undef INIT_TYPE
}
