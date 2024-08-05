#pragma once

#include "pycore_atomic.h"
#include "pycore_initconfig.h"
#include "cpython/initconfig.h"
typedef struct _is PyInterpreterState;

/* GIL state */
struct _gilstate_runtime_state {
  // Assuming the current thread holds the GIL, this is the
  // PyThreadState for the current thread. */
  _Py_atomic_address tstate_current;
};

// Full Python runtime state

typedef struct pyruntimestate {
  // is running Py_PreInitialize()?
  int preinitializing;
  int preinitialized;

  int core_initialized;

  int initialized;
  struct _gilstate_runtime_state gilstate;

	struct pyinterpreters {
		PyInterpreterState *main;
	} interpreters;
} _PyRuntimeState;

#define _PyRuntimeState_INIT \
  {.preinitialized = 0, .core_initialized = 0, .initialized = 0}

static PyStatus
_PyRuntimeState_Init_impl(_PyRuntimeState *runtime) {
  memset(runtime, 0, sizeof(*runtime));
  return _PyStatus_OK();
}

// defined in cpy/Python/pystate.c
PyStatus _PyRuntimeState_Init(_PyRuntimeState *runtime) {
  return _PyRuntimeState_Init_impl(runtime);
}

// declared in cpy/Include/internal/pycore_runtime.h
// defined in cpy/Python/pylifecycle.c
_PyRuntimeState _PyRuntime = _PyRuntimeState_INIT;

static int runtime_initialized = 0;

// Initialize _PyRuntimeState.
// Return NULL on success, or return an error message on failure.
// Defined in cpy/Python/pylifecycle.c
PyStatus _PyRuntime_Initialize(void) {
  if (runtime_initialized) {
    return _PyStatus_OK();
  }
  runtime_initialized = 1;
  return _PyRuntimeState_Init(&_PyRuntime);
}


