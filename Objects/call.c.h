#pragma once

PyObject *
_Py_CheckFunctionResult(PyThreadState *tstate, PyObject *callable,
		PyObject *result, const char *where) {
	assert((callable != NULL) ^ (where != NULL));

	if (result == NULL) {
		assert(false);
	} else {
		if (_PyErr_Occurred(tstate)) {
			assert(false);
		}
	}
	return result;
}

PyObject *_PyFunction_Vectorcall(PyObject *func, PyObject *const *stack, size_t nargsf, PyObject *kwnames) {
	assert(PyFunction_Check(func));
	PyFrameConstructor *f = PyFunction_AS_FRAME_CONSTRUCTOR(func);
	Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
	// printf("nargs %ld\n", nargs);
	assert(nargs >= 0);
	PyThreadState *tstate = _PyThreadState_GET();
	assert(nargs == 0 || stack != NULL);
	if (((PyCodeObject *)f->fc_code)->co_flags & CO_OPTIMIZED) {
		return _PyEval_Vector(tstate, f, NULL, stack, nargs, kwnames);
	} else {
		assert(false);
	}
}


PyObject *PyVectorcall_Call(PyObject *callable, PyObject *tuple, PyObject *kwargs) {
  PyThreadState *tstate = _PyThreadState_GET();
  vectorcallfunc func;

  Py_ssize_t offset = Py_TYPE(callable)->tp_vectorcall_offset;
  if (offset <= 0) {
    assert(false);
  }
  memcpy(&func, (char *) callable + offset, sizeof(func));
  if (func == NULL) {
    assert(false);
  }

  Py_ssize_t nargs = PyTuple_GET_SIZE(tuple);

  // Fast path for no keywords
  if (kwargs == NULL || PyDict_GET_SIZE(kwargs) == 0) {
    return func(callable, _PyTuple_ITEMS(tuple), nargs, NULL);
  }
	assert(false);
}

PyObject *
_PyObject_FastCallDictTstate(PyThreadState *tstate, PyObject *callable,
    PyObject *const *args, size_t nargsf,
    PyObject *kwargs) {
  assert(callable != NULL);
  assert(!_PyErr_Occurred(tstate));
  
  Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
  assert(nargs >= 0);
  assert(nargs == 0 || args != NULL);
  assert(kwargs == NULL || PyDict_Check(kwargs));

  vectorcallfunc func = PyVectorcall_Function(callable);
  // printf("_PyObject_FastCallDictTstate vectorcallfunc is %p, type %s\n", func, Py_TYPE(callable)->tp_name);
  if (func == NULL) {
    // Use tp_call instead
    assert(false);
  }

  PyObject *res;
  if (kwargs == NULL || PyDict_GET_SIZE(kwargs) == 0) {
    res = func(callable, args, nargsf, NULL);
  } else {
    assert(false);
  }

  return _Py_CheckFunctionResult(tstate, callable, res, NULL);
}

PyObject *PyObject_VectorcallDict(PyObject *callable, PyObject *const *args, size_t nargsf, PyObject *kwargs) {
  PyThreadState *tstate = _PyThreadState_GET();
  return _PyObject_FastCallDictTstate(tstate, callable, args, nargsf, kwargs);
}

// type(...) call this to create new class type.
PyObject *_PyObject_MakeTpCall(PyThreadState *tstate, PyObject *callable,
    PyObject *const *args, Py_ssize_t nargs,
    PyObject *keywords) {
  ternaryfunc call = Py_TYPE(callable)->tp_call;
  if (call == NULL) {
    assert(false);
  }

  PyObject *argstuple = _PyTuple_FromArray(args, nargs);
  if (argstuple == NULL) {
    return NULL;
  }

  PyObject *kwdict;
  if (keywords == NULL || PyDict_Check(keywords)) {
    kwdict = keywords;
  } else {
    assert(false);
  }

  PyObject *result = NULL;

  result = call(callable, argstuple, kwdict);

  Py_DECREF(argstuple);
  if (kwdict != keywords) {
    Py_DECREF(kwdict);
  }
  return _Py_CheckFunctionResult(tstate, callable, result, NULL);
}


PyObject *_PyObject_Call(PyThreadState *tstate, PyObject *callable, PyObject *args, PyObject *kwargs) {
  assert(!_PyErr_Occurred(tstate));
  assert(PyTuple_Check(args));
  assert(kwargs == NULL || PyDict_Check(kwargs));

  if (PyVectorcall_Function(callable) != NULL) {
    return PyVectorcall_Call(callable, args, kwargs);
  } else {
    assert(false);
  }
}

// call callable(obj, *args, **kwargs)
PyObject *_PyObject_Call_Prepend(PyThreadState *tstate, PyObject *callable,
    PyObject *obj, PyObject *args, PyObject *kwargs) {
  assert(PyTuple_Check(args));

  PyObject *small_stack[_PY_FASTCALL_SMALL_STACK];
  PyObject **stack;

  Py_ssize_t argcount = PyTuple_GET_SIZE(args);
  if (argcount + 1 <= (Py_ssize_t) _PY_FASTCALL_SMALL_STACK) {
    stack = small_stack;
  } else {
    assert(false);
  }

  stack[0] = obj;
  memcpy(&stack[1],
    _PyTuple_ITEMS(args),
    argcount * sizeof(PyObject *));
  PyObject *result = _PyObject_FastCallDictTstate(tstate, callable,
      stack, argcount + 1, kwargs);
  if (stack != small_stack) {
    PyMem_Free(stack);
  }
  return result;
}
