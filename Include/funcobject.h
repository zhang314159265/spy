#pragma once

#define COMMON_FIELDS(PREFIX) \
	PyObject *PREFIX ## globals; \
	PyObject *PREFIX ## builtins; \
	PyObject *PREFIX ## name; \
	PyObject *PREFIX ## qualname; \
	PyObject *PREFIX ## code; /* A code object, the __code__ attribute */ \
	PyObject *PREFIX ## defaults; \
	PyObject *PREFIX ## kwdefaults; \
	PyObject *PREFIX ## closure;

#define PyFunction_AS_FRAME_CONSTRUCTOR(func) \
		((PyFrameConstructor *) &((PyFunctionObject *) (func))->func_globals)

#define PyFunction_Check(op) Py_IS_TYPE(op, &PyFunction_Type)

typedef struct {
	COMMON_FIELDS(fc_)
} PyFrameConstructor;

typedef struct {
	PyObject_HEAD
	COMMON_FIELDS(func_)

	PyObject *func_doc;
	PyObject *func_dict;

	PyObject *func_module;
	vectorcallfunc vectorcall;
} PyFunctionObject;

PyObject *PyVectorcall_Call(PyObject *callable, PyObject *tuple, PyObject *kwargs);

PyTypeObject PyFunction_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "function",
	.tp_basicsize = sizeof(PyFunctionObject),
	.tp_flags = Py_TPFLAGS_HAVE_VECTORCALL,
	.tp_call = PyVectorcall_Call,
	.tp_vectorcall_offset = offsetof(PyFunctionObject, vectorcall),
};

PyObject *_PyEval_BuiltinsFromGlobals(PyThreadState *tstate, PyObject *globals);

PyObject *_PyFunction_Vectorcall(PyObject *func, PyObject *const *stack, size_t nargsf, PyObject *kwnames);

// defined in cpy/Objects/funcobject.c
PyObject *PyFunction_NewWithQualName(PyObject *code, PyObject *globals, PyObject *qualname) {
	assert(globals != NULL);
	assert(PyDict_Check(globals));
	Py_INCREF(globals);

	PyThreadState *tstate = _PyThreadState_GET();

	PyCodeObject *code_obj = (PyCodeObject *) code;
	Py_INCREF(code_obj);

	PyObject *name = code_obj->co_name;
	assert(name != NULL);
	Py_INCREF(name);
	if (!qualname) {
		qualname = name;
	}
	Py_INCREF(qualname);

	PyObject *consts = code_obj->co_consts;
	assert(PyTuple_Check(consts));
	PyObject *doc;
	if (PyTuple_Size(consts) >= 1) {
		doc = PyTuple_GetItem(consts, 0);
		if (!PyUnicode_Check(doc)) {
			doc = Py_None;
		}
	} else {
		doc = Py_None;
	}
	Py_INCREF(doc);

	_Py_IDENTIFIER(__name__);
	PyObject *module = _PyDict_GetItemIdWithError(globals, &PyId___name__);
	PyObject *builtins = NULL;
	if (module == NULL && _PyErr_Occurred(tstate)) {
		assert(false);
	}
	Py_XINCREF(module);

	builtins = _PyEval_BuiltinsFromGlobals(tstate, globals);

	if (builtins == NULL) {
		assert(false);
	}
	Py_INCREF(builtins);

	PyFunctionObject *op = PyObject_GC_New(PyFunctionObject, &PyFunction_Type);
	if (op == NULL) {
		assert(false);
	}

	op->func_globals = globals;
	op->func_builtins = builtins;
	op->func_name = name;
	op->func_qualname = qualname;

	op->func_code = (PyObject *) code_obj;
	op->func_defaults = NULL;
	op->func_kwdefaults = NULL;
	op->func_closure = NULL;
	op->func_doc = doc;
	op->func_dict = NULL;
	op->func_module = module;
	op->vectorcall = _PyFunction_Vectorcall;

	return (PyObject *) op;
	assert(false);
}
