#pragma once

typedef struct {
	PyObject_HEAD
	PyTypeObject *d_type;
	PyObject *d_name;
	PyObject *d_qualname;
} PyDescrObject;

#define PyDescr_COMMON PyDescrObject d_common

typedef struct {
	PyDescr_COMMON;
	PyMethodDef *d_method;
	vectorcallfunc vectorcall;
} PyMethodDescrObject;

typedef void (*funcptr)(void);

static inline funcptr
method_enter_call(PyThreadState *tstate, PyObject *func) {
	return (funcptr) ((PyMethodDescrObject *) func)->d_method->ml_meth;
}

static PyObject *
method_vectorcall_VARARGS_KEYWORDS(
		PyObject *func, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
	PyThreadState *tstate = _PyThreadState_GET();
	Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);

	PyObject *argstuple = _PyTuple_FromArray(args + 1, nargs - 1);
	if (argstuple == NULL) {
		return NULL;
	}
	PyObject *result = NULL;
	PyObject *kwdict = NULL;
	if (kwnames != NULL && PyTuple_GET_SIZE(kwnames) > 0) {
		assert(false);
	}

	PyCFunctionWithKeywords meth = (PyCFunctionWithKeywords)
			method_enter_call(tstate, func);
	if (meth == NULL) {
		assert(false);
	}
	result = meth(args[0], argstuple, kwdict);
exit:
	Py_DECREF(argstuple);
	Py_XDECREF(kwdict);
	return result;
}

PyTypeObject PyMethodDescr_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "method_descriptor",
	.tp_basicsize = sizeof(PyMethodDescrObject),
	.tp_flags = Py_TPFLAGS_HAVE_VECTORCALL | Py_TPFLAGS_METHOD_DESCRIPTOR,
	.tp_call = PyVectorcall_Call,
	.tp_vectorcall_offset = offsetof(PyMethodDescrObject, vectorcall),
};

static PyDescrObject *
descr_new(PyTypeObject *descrtype, PyTypeObject *type, const char *name) {
	PyDescrObject *descr;

	descr = (PyDescrObject *) PyType_GenericAlloc(descrtype, 0);
	if (descr != NULL) {
		Py_XINCREF(type);
		descr->d_type = type;
		descr->d_name = PyUnicode_InternFromString(name);
		if (descr->d_name == NULL) {
			Py_DECREF(descr);
			descr = NULL;
		} else {
			descr->d_qualname = NULL;
		}
	}
	return descr;
}

static PyObject *
method_vectorcall_O(
		PyObject *func, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
	PyThreadState *tstate = _PyThreadState_GET();
	Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
	if (nargs != 2) {
		assert(false);
	}
	PyCFunction meth = (PyCFunction) method_enter_call(tstate, func);
	if (meth == NULL) {
		return NULL;
	}
	PyObject *result = meth(args[0], args[1]);
	return result;
}

PyObject *
PyDescr_NewMethod(PyTypeObject *type, PyMethodDef *method) {
	vectorcallfunc vectorcall;
	switch (method->ml_flags & (METH_VARARGS | METH_FASTCALL | METH_NOARGS |
			METH_O | METH_KEYWORDS | METH_METHOD)) {
	case METH_VARARGS | METH_KEYWORDS:
		vectorcall = method_vectorcall_VARARGS_KEYWORDS;
		break;
	case METH_O:
		vectorcall = method_vectorcall_O;
		break;
	default:
		assert(false);
	}

	PyMethodDescrObject *descr;

	descr = (PyMethodDescrObject *) descr_new(
			&PyMethodDescr_Type, type, method->ml_name);
	if (descr != NULL) {
		descr->d_method = method;
		descr->vectorcall = vectorcall;
	}
	return (PyObject *) descr;
}

#define PyDescr_NAME(x) (((PyDescrObject *)(x))->d_name)
