#pragma once

PyFrameObject *
_PyFrame_New_NoTrack(PyThreadState *tstate, PyFrameConstructor *con, PyObject *locals) {
	PyFrameObject *f = frame_alloc((PyCodeObject *) con->fc_code);
	if (f == NULL) {
		return NULL;
	}
	f->f_back = (PyFrameObject*) Py_XNewRef(tstate->frame);
	f->f_code = (PyCodeObject *) Py_NewRef(con->fc_code);
	f->f_builtins = Py_NewRef(con->fc_builtins);
	f->f_globals = Py_NewRef(con->fc_globals);
	f->f_locals = Py_XNewRef(locals);
	f->f_stackdepth = 0;
	f->f_lasti = -1;
	// f_valuestack is already initialized in frame_alloc
	f->f_state = FRAME_CREATED;
	return f;
}
