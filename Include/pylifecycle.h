#pragma once

// cpy does not have this header file. Put functions in cpy/Python/pylifecycle.c here

static PyStatus
pycore_init_types(PyInterpreterState *interp) {
	PyStatus status;
	int is_main_interp = _Py_IsMainInterpreter(interp);
	if (is_main_interp) {
		status = _PyTypes_Init();
		if (_PyStatus_EXCEPTION(status)) {
			return status;
		}
	}
	return _PyStatus_OK();
}
