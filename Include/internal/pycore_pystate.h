#include "pycore_runtime.h"
#include "cpython/pystate.h"

static inline PyThreadState *
_PyRuntimeState_GetThreadState(_PyRuntimeState *runtime) {
  return (PyThreadState*) _Py_atomic_load_relaxed(&runtime->gilstate.tstate_current);
}

static inline PyThreadState *
_PyThreadState_GET(void) {
  return _PyRuntimeState_GetThreadState(&_PyRuntime);
}

static inline PyInterpreterState *_PyInterpreterState_GET(void) {
  PyThreadState *tstate = _PyThreadState_GET();
  return tstate->interp;
}

static inline int
_Py_IsMainInterpreter(PyInterpreterState *interp) {
	// Use directly _PyRuntime rather than tstate->interp->runtime, since
	// this function is used in performance critical code path (eeval) */
	return (interp == _PyRuntime.interpreters.main);
}
