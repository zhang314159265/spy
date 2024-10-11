#pragma once

static inline PyObject *_PyErr_Occurred(PyThreadState *tstate)
{
	assert(tstate != NULL);
	return tstate->curexc_type;
}


