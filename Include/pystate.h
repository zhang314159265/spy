#pragma once

#include "cpython/pymem.h"
#include "internal/pycore_runtime.h"

// struct _ts is defined in Include/cpython/pystate.h
typedef struct _ts PyThreadState;

// struct _is is define in internal/pycore_interp.h
typedef struct _is PyInterpreterState;

// #include "internal/pycore_pystate.h"
static PyThreadState *_PyThreadState_GET(void);

#define PyThreadState_GET() PyThreadState_Get()

PyThreadState *
PyThreadState_Get(void) {
	PyThreadState *tstate = _PyThreadState_GET();
	assert(tstate != NULL);
	return tstate;
}

PyObject * _PyEval_EvalFrameDefault(PyThreadState *tstate, PyFrameObject *f, int throwflag);

// defined in cpy/Python/pystate.c
PyInterpreterState *PyInterpreterState_New() {
  PyThreadState *tstate = _PyThreadState_GET();

  /* tstate is NULL when Py_InitializeFromConfig() calls
     PyInterpreterState_New() to create the main interpreter. */
  PyInterpreterState *interp = PyMem_RawCalloc(1, sizeof(PyInterpreterState));
  if (interp == NULL) {
    return NULL;
  }

  // Don't get runtine from tstate since tstate can be NULL
  _PyRuntimeState *runtime = &_PyRuntime;
  interp->runtime = runtime;

	interp->eval_frame = _PyEval_EvalFrameDefault;

	struct pyinterpreters *interpreters = &runtime->interpreters;
	if (interpreters->main == NULL) {
		interpreters->main = interp;
	}
  return interp;
}

#include "cpython/pystate.h"

static void
_PyGILState_NoteThreadState(struct _gilstate_runtime_state *gilstate, PyThreadState *tstate) {
  // nop for now
}

void
_PyThreadState_Init(PyThreadState *tstate) {
  _PyGILState_NoteThreadState(&tstate->interp->runtime->gilstate, tstate);
}

static PyThreadState *
new_threadstate(PyInterpreterState *interp, int init) {
  _PyRuntimeState *runtime = interp->runtime;
  PyThreadState *tstate = (PyThreadState *) PyMem_RawMalloc(sizeof(PyThreadState));
  if (tstate == NULL) {
    return NULL;
  }
  tstate->interp = interp;

  tstate->frame = NULL;

	tstate->curexc_type = NULL;

  if (init) {
    _PyThreadState_Init(tstate);
  }
  return tstate;
}

// defined in cpy/Python/pystate.c
PyThreadState *
PyThreadState_New(PyInterpreterState *interp) {
  return new_threadstate(interp, 1);
}

#define _PyRuntimeGILState_GetThreadState(gilstate) \
  ((PyThreadState*) _Py_atomic_load_relaxed(&(gilstate)->tstate_current))

#define _PyRuntimeGILState_SetThreadState(gilstate, value) \
  _Py_atomic_store_relaxed(&(gilstate)->tstate_current, \
    (uintptr_t) (value))

PyThreadState *
_PyThreadState_Swap(struct _gilstate_runtime_state *gilstate, PyThreadState *newts) {
  PyThreadState *oldts = _PyRuntimeGILState_GetThreadState(gilstate);
  _PyRuntimeGILState_SetThreadState(gilstate, newts);
  return oldts;
}

// defined in cpy/Python/pystate.c
PyThreadState *PyThreadState_Swap(PyThreadState *newts) {
  return _PyThreadState_Swap(&_PyRuntime.gilstate, newts);
}
