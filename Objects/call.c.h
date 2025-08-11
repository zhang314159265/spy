#pragma once

PyObject *
_Py_CheckFunctionResult(PyThreadState *tstate, PyObject *callable,
		PyObject *result, const char *where) {
	assert((callable != NULL) ^ (where != NULL));

	if (result == NULL) {
    if (!_PyErr_Occurred(tstate)) {
      fail(0);
    }
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

static PyObject *const * _PyStack_UnpackDict(PyThreadState *tstate, PyObject *const *args, Py_ssize_t nargs, PyObject *kwargs, PyObject **p_kwnames);

static void _PyStack_UnpackDict_Free(PyObject *const *stack, Py_ssize_t nargs, PyObject *kwnames);


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

  PyObject *const *args;
  PyObject *kwnames;
  args = _PyStack_UnpackDict(tstate, _PyTuple_ITEMS(tuple), nargs,
      kwargs, &kwnames);
  if (args == NULL) {
    return NULL;
  }
  PyObject *result = func(callable, args,
      nargs | PY_VECTORCALL_ARGUMENTS_OFFSET, kwnames);
  _PyStack_UnpackDict_Free(args, nargs, kwnames);
  return _Py_CheckFunctionResult(tstate, callable, result, NULL);
}

static PyObject *const *
_PyStack_UnpackDict(PyThreadState *tstate,
    PyObject *const *args, Py_ssize_t nargs,
    PyObject *kwargs, PyObject **p_kwnames) {
  assert(nargs >= 0);
  assert(kwargs != NULL);
  assert(PyDict_Check(kwargs));

  Py_ssize_t nkwargs = PyDict_GET_SIZE(kwargs);

  PyObject **stack = PyMem_Malloc((1 + nargs + nkwargs) * sizeof(args[0]));
  if (stack == NULL) {
    fail(0);
  }

  PyObject *kwnames = PyTuple_New(nkwargs);
  if (kwnames == NULL) {
    fail(0);
  }
  stack++;

  for (Py_ssize_t i = 0; i < nargs; i++) {
    Py_INCREF(args[i]);
    stack[i] = args[i];
  }

  PyObject **kwstack = stack + nargs;
  Py_ssize_t pos = 0, i = 0;
  PyObject *key, *value;

  unsigned long keys_are_strings = Py_TPFLAGS_UNICODE_SUBCLASS;
  while (PyDict_Next(kwargs, &pos, &key, &value)) {
    keys_are_strings &= Py_TYPE(key)->tp_flags;
    Py_INCREF(key);
    Py_INCREF(value);
    PyTuple_SET_ITEM(kwnames, i, key);
    kwstack[i] = value;
    i++;
  }

  if (!keys_are_strings) {
    fail(0);
  }
  *p_kwnames = kwnames;
  return stack;
}

static void
_PyStack_UnpackDict_Free(PyObject *const *stack, Py_ssize_t nargs,
    PyObject *kwnames) {
  Py_ssize_t n = PyTuple_GET_SIZE(kwnames) + nargs;
  for (Py_ssize_t i = 0; i < n; i++) {
    Py_DECREF(stack[i]);
  }
  PyMem_Free((PyObject **) stack - 1);
  Py_DECREF(kwnames);
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
    return _PyObject_MakeTpCall(tstate, callable, args, nargs, kwargs);
  }

  PyObject *res;
  if (kwargs == NULL || PyDict_GET_SIZE(kwargs) == 0) {
    res = func(callable, args, nargsf, NULL);
  } else {
    PyObject *kwnames;
    PyObject *const *newargs;
    newargs = _PyStack_UnpackDict(tstate,
        args, nargs, kwargs, &kwnames);
    if (newargs == NULL) {
      return NULL;
    }
    res = func(callable, newargs,
        nargs | PY_VECTORCALL_ARGUMENTS_OFFSET, kwnames);
    _PyStack_UnpackDict_Free(newargs, nargs, kwnames);
  }

  return _Py_CheckFunctionResult(tstate, callable, res, NULL);
}

PyObject *PyObject_VectorcallDict(PyObject *callable, PyObject *const *args, size_t nargsf, PyObject *kwargs) {
  PyThreadState *tstate = _PyThreadState_GET();
  return _PyObject_FastCallDictTstate(tstate, callable, args, nargsf, kwargs);
}

PyObject *
_PyStack_AsDict(PyObject *const *values, PyObject *kwnames) {
  Py_ssize_t nkwargs;
  PyObject *kwdict;
  Py_ssize_t i;

  assert(kwnames != NULL);
  nkwargs = PyTuple_GET_SIZE(kwnames);
  kwdict = _PyDict_NewPresized(nkwargs);
  if (kwdict == NULL) {
    return NULL;
  }

  for (i = 0; i < nkwargs; i++) {
    PyObject *key = PyTuple_GET_ITEM(kwnames, i);
    PyObject *value = *values++;
    if (PyDict_SetItem(kwdict, key, value)) {
      Py_DECREF(kwdict);
      return NULL;
    }
  }
  return kwdict;
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
    if (PyTuple_GET_SIZE(keywords)) {
      assert(args != NULL);
      kwdict = _PyStack_AsDict(args + nargs, keywords);
      if (kwdict == NULL) {
        fail(0);
      }
    } else {
      keywords = kwdict = NULL;
    }
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
    ternaryfunc call = Py_TYPE(callable)->tp_call;
    if (call == NULL) {
      fail("tp_call is null for type %s\n", Py_TYPE(callable)->tp_name);
      return NULL;
    }

    PyObject *result = (*call)(callable, args, kwargs);

    return _Py_CheckFunctionResult(tstate, callable, result, NULL);
  }
}

PyObject *PyObject_Call(PyObject *callable, PyObject *args, PyObject *kwargs) {
  PyThreadState *tstate = _PyThreadState_GET();
  return _PyObject_Call(tstate, callable, args, kwargs);
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

static PyObject *
object_vacall(PyThreadState *tstate, PyObject *base,
    PyObject *callable, va_list vargs)
{
  PyObject *small_stack[_PY_FASTCALL_SMALL_STACK];
  PyObject **stack;
  Py_ssize_t nargs;
  PyObject *result;
  Py_ssize_t i;
  va_list countva;

  if (callable == NULL) {
    assert(false);
  }

  va_copy(countva, vargs);
  nargs = base ? 1 : 0;
  while (1) {
    PyObject *arg = va_arg(countva, PyObject *);
    if (arg == NULL) {
      break;
    }
    nargs++;
  }
  va_end(countva);

  if (nargs <= (Py_ssize_t) Py_ARRAY_LENGTH(small_stack)) {
    stack = small_stack;
  } else {
    assert(false);
  }

  i = 0;
  if (base) {
    stack[i++] = base;
  }
  for (; i < nargs; ++i) {
    stack[i] = va_arg(vargs, PyObject *);
  }

  result = _PyObject_VectorcallTstate(tstate, callable, stack, nargs, NULL);
  if (stack != small_stack) {
    PyMem_Free(stack);
  }
  return result;
}

PyObject *
PyObject_CallFunctionObjArgs(PyObject *callable, ...) {
  PyThreadState *tstate = _PyThreadState_GET();
  va_list vargs;
  PyObject *result;

  va_start(vargs, callable);
  result = object_vacall(tstate, NULL, callable, vargs);
  va_end(vargs);

  return result;
}

PyObject *_PyObject_CallMethodIdObjArgs(PyObject *obj, struct _Py_Identifier *name, ...) {
  PyThreadState *tstate = _PyThreadState_GET();
  if (obj == NULL || name == NULL) {
    assert(false);
  }

  PyObject *oname = _PyUnicode_FromId(name);
  if (!oname) {
    return NULL;
  }

  PyObject *callable = NULL;
  int is_method = _PyObject_GetMethod(obj, oname, &callable);
  if (callable == NULL) {
    return NULL;
  }
  obj = is_method ? obj : NULL;

  va_list vargs;
  va_start(vargs, name);
  PyObject *result = object_vacall(tstate, obj, callable, vargs);
  va_end(vargs);

  Py_DECREF(callable);
  return result;
}


static PyObject *
_PyObject_CallFunctionVa(PyThreadState *tstate, PyObject *callable,
    const char *format, va_list va, int is_size_t) {
  PyObject *small_stack[_PY_FASTCALL_SMALL_STACK];
  const Py_ssize_t small_stack_len = Py_ARRAY_LENGTH(small_stack);
  PyObject **stack;
  Py_ssize_t nargs, i;
  PyObject *result;

  if (callable == NULL) {
    fail(0);
  }

  if (!format || !*format) {
    return _PyObject_CallNoArgTstate(tstate, callable);
  }

  if (is_size_t) {
    fail(0);
  } else {
    stack = _Py_VaBuildStack(small_stack, small_stack_len, format, va, &nargs);
  }
  if (stack == NULL) {
    return NULL;
  }

  if (nargs == 1 && PyTuple_Check(stack[0])) {
    fail(0);
  } else {
    result = _PyObject_VectorcallTstate(tstate, callable, stack, nargs, NULL);
  }

  for (i = 0; i < nargs; ++i) {
    Py_DECREF(stack[i]);
  }
  if (stack != small_stack) {
    PyMem_Free(stack);
  }
  return result;
}

PyObject * _PyObject_CallMethodId(PyObject *obj, _Py_Identifier *name,
    const char *format, ...) {
  PyThreadState *tstate = _PyThreadState_GET();
  if (obj == NULL || name == NULL) {
    fail(0);
  }

  PyObject *callable = _PyObject_GetAttrId(obj, name);
  if (callable == NULL)
    return NULL;

  va_list va;
  va_start(va, format);
  PyObject *retval = callmethod(tstate, callable, format, va, 0);
  va_end(va);

  Py_DECREF(callable);
  return retval;
}

PyObject *PyObject_CallFunction(PyObject *callable, const char *format, ...) {
  va_list va;
  PyObject *result;
  PyThreadState *tstate = _PyThreadState_GET();

  va_start(va, format);
  result = _PyObject_CallFunctionVa(tstate, callable, format, va, 0);
  va_end(va);

  return result;
}
