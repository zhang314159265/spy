#pragma once

typedef PyObject *(*PyCFunction)(PyObject *, PyObject *);
typedef PyObject *(*PyCFunctionWithKeywords)(PyObject *, PyObject *, PyObject *);

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
  PyObject *m_weakreflist;
	vectorcallfunc vectorcall;
} PyCFunctionObject;

typedef struct {
  PyCFunctionObject func;
  PyTypeObject *mm_class;
} PyCMethodObject;

static PyObject *cfunction_call(PyObject *func, PyObject *args, PyObject *kwargs);
static void meth_dealloc(PyCFunctionObject *m);

PyTypeObject PyCFunction_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "builtin_function_or_method",
	.tp_basicsize = sizeof(PyCFunctionObject),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_HAVE_VECTORCALL,
	.tp_call = cfunction_call,
	.tp_vectorcall_offset = offsetof(PyCFunctionObject, vectorcall),
  .tp_dealloc = (destructor) meth_dealloc,
};

#define METH_VARARGS 0x0001
#define METH_KEYWORDS 0x0002
#define METH_NOARGS 0x0004
#define METH_O 0x0008
#define METH_CLASS 0x0010
#define METH_STATIC 0x0020
#define METH_COEXIST 0x0040
#define METH_METHOD 0x0200
#define METH_FASTCALL 0x0080

typedef void (*funcptr)(void);

#define PyCFunction_GET_CLASS(func) \
    (((PyCFunctionObject *) func)->m_ml->ml_flags & METH_METHOD ? \
      ((PyCMethodObject *) func)->mm_class : NULL)

#define PyCFunction_GET_FUNCTION(func) \
		(((PyCFunctionObject *) func)->m_ml->ml_meth)

#define PyCFunction_GET_SELF(func) \
		(((PyCFunctionObject *) func)->m_ml->ml_flags & METH_STATIC ? \
			NULL : ((PyCFunctionObject *) func)->m_self)

#define PyCFunction_Check(op) PyObject_TypeCheck(op, &PyCFunction_Type)

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

static PyObject *
cfunction_vectorcall_O(
		PyObject *func, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
	PyThreadState *tstate = _PyThreadState_GET();
	Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
	if (nargs != 1) {
		assert(false);
	}
	PyCFunction meth = (PyCFunction) cfunction_enter_call(tstate, func);
	if (meth == NULL) {
		return NULL;
	}
	PyObject *result = meth(PyCFunction_GET_SELF(func), args[0]);
	return result;
}

static inline int
cfunction_check_kwargs(PyThreadState *tstate, PyObject *func, PyObject *kwnames) {
  assert(!_PyErr_Occurred(tstate));
  assert(PyCFunction_Check(func));

  if (kwnames && PyTuple_GET_SIZE(kwnames)) {
    assert(false);
  }
  return 0;
}

static PyObject *
cfunction_vectorcall_NOARGS(
    PyObject *func, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
  PyThreadState *tstate = _PyThreadState_GET();
  if (cfunction_check_kwargs(tstate, func, kwnames)) {
    return NULL;
  }
  Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
  if (nargs != 0) {
    assert(false);
  }
  PyCFunction meth = (PyCFunction) cfunction_enter_call(tstate, func);
  if (meth == NULL) {
    return NULL;
  }
  PyObject *result = meth(PyCFunction_GET_SELF(func), NULL);
  return result;
}

PyObject *
PyCMethod_New(PyMethodDef *ml, PyObject *self, PyObject *module, PyTypeObject *cls) {
	vectorcallfunc vectorcall;
	// printf("module variable type %s\n", Py_TYPE(module)->tp_name);
	switch (ml->ml_flags & (METH_VARARGS | METH_FASTCALL | METH_NOARGS |
			METH_O | METH_KEYWORDS | METH_METHOD)) {
  case METH_VARARGS | METH_KEYWORDS:
    // use tp_call instead of vectorcall
    vectorcall = NULL;
    break;
	case METH_FASTCALL | METH_KEYWORDS:
		vectorcall = cfunction_vectorcall_FASTCALL_KEYWORDS;
		break;
  case METH_NOARGS:
    vectorcall = cfunction_vectorcall_NOARGS;
    break;
	case METH_O:
		vectorcall = cfunction_vectorcall_O;
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

  op->m_weakreflist = NULL;
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

static void meth_dealloc(PyCFunctionObject *m) {
  // PyObject_GC_UnTrack(m);
  if (m->m_weakreflist != NULL) {
    assert(false);
  }
  Py_XDECREF(PyCFunction_GET_CLASS(m));
  Py_XDECREF(m->m_self);
  Py_XDECREF(m->m_module);
  PyObject_GC_Del(m);
}
