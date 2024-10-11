#pragma once

typedef PyObject *(*PyCFunction)(PyObject *, PyObject *);

typedef PyObject *(*_PyCFunctionFastWithKeywords)(PyObject *,
		PyObject *const *,
		Py_ssize_t,
		PyObject *);

struct PyMethodDef {
	const char *ml_name;
	PyCFunction ml_meth;
	int ml_flags;
	const char *ml_doc;
};
typedef struct PyMethodDef PyMethodDef;

// defined in cpy/Include/cpython/methodobject.h
typedef struct {
	PyObject_HEAD
	PyMethodDef *m_ml;
	PyObject *m_self;
	PyObject *m_module; // The __module__ attribute, can be anything
	vectorcallfunc vectorcall;
} PyCFunctionObject;

static PyObject *cfunction_call(PyObject *func, PyObject *args, PyObject *kwargs);

PyTypeObject PyCFunction_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "builtin_function_or_method",
	.tp_basicsize = sizeof(PyCFunctionObject),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_HAVE_VECTORCALL,
	.tp_call = cfunction_call,
	.tp_vectorcall_offset = offsetof(PyCFunctionObject, vectorcall),
};

#define METH_VARARGS 0x0001
#define METH_KEYWORDS 0x0002
#define METH_NOARGS 0x0004
#define METH_O 0x0008
#define METH_STATIC 0x0020
#define METH_METHOD 0x0200
#define METH_FASTCALL 0x0080

typedef void (*funcptr)(void);

#define PyCFunction_GET_FUNCTION(func) \
		(((PyCFunctionObject *) func)->m_ml->ml_meth)

#define PyCFunction_GET_SELF(func) \
		(((PyCFunctionObject *) func)->m_ml->ml_flags & METH_STATIC ? \
			NULL : ((PyCFunctionObject *) func)->m_self)

static inline funcptr
cfunction_enter_call(PyThreadState *tstate, PyObject *func) {
	return (funcptr) PyCFunction_GET_FUNCTION(func);
}

static PyObject *
cfunction_vectorcall_FASTCALL_KEYWORDS(
		PyObject *func, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
	PyThreadState *tstate = _PyThreadState_GET();
	Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
	_PyCFunctionFastWithKeywords meth = (_PyCFunctionFastWithKeywords)
			cfunction_enter_call(tstate, func);
	if (meth == NULL) {
		return NULL;
	}
	PyObject *result = meth(PyCFunction_GET_SELF(func), args, nargs, kwnames);
	return result;
}

PyObject *
PyCMethod_New(PyMethodDef *ml, PyObject *self, PyObject *module, PyTypeObject *cls) {
	vectorcallfunc vectorcall;
	// printf("module variable type %s\n", Py_TYPE(module)->tp_name);
	switch (ml->ml_flags & (METH_VARARGS | METH_FASTCALL | METH_NOARGS |
			METH_O | METH_KEYWORDS | METH_METHOD)) {
	case METH_FASTCALL | METH_KEYWORDS:
		vectorcall = cfunction_vectorcall_FASTCALL_KEYWORDS;
		break;
	default:
		assert(false);
	}

	PyCFunctionObject *op = NULL;

	if (ml->ml_flags & METH_METHOD) {
		assert(false);
	} else {
		if (cls) {
			assert(false);
		}
		op = PyObject_GC_New(PyCFunctionObject, &PyCFunction_Type);
		if (op == NULL) {
			return NULL;
		}
	}
	op->m_ml = ml;
	Py_XINCREF(self);
	op->m_self = self;
	Py_XINCREF(module);
	op->m_module = module;
	op->vectorcall = vectorcall;
	_PyObject_GC_TRACK(op);
	return (PyObject *) op;
}

PyObject *PyCFunction_NewEx(PyMethodDef *ml, PyObject *self, PyObject *module) {
	return PyCMethod_New(ml, self, module, NULL);
}

static PyObject *cfunction_call(PyObject *func, PyObject *args, PyObject *kwargs) {
	assert(false);
}
