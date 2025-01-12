#pragma once

#include "moduleobject.h"

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
	// printf("in _PyFrame_New_NoTrack name %s f_locals is %p\n", (char*) PyUnicode_DATA(((PyCodeObject *) (con->fc_code))->co_name), f->f_locals);
	f->f_stackdepth = 0;
  f->f_gen = NULL;
	f->f_lasti = -1;
  f->f_iblock = 0;
	// f_valuestack is already initialized in frame_alloc
	f->f_state = FRAME_CREATED;
	return f;
}

void PyFrame_BlockSetup(PyFrameObject *f, int type, int handler, int level) {
  PyTryBlock *b;
  if (f->f_iblock >= CO_MAXBLOCKS) {
    assert(false);
  }
  b = &f->f_blockstack[f->f_iblock++];
  b->b_type = type;
  b->b_level = level;
  b->b_handler = handler;
}

PyTryBlock *PyFrame_BlockPop(PyFrameObject *f) {
  PyTryBlock *b;
  if (f->f_iblock <= 0) {
    assert(false);
  }
  b = &f->f_blockstack[--f->f_iblock];
  return b;
}
PyObject *_PyEval_BuiltinsFromGlobals(PyThreadState *tstate, PyObject *globals)
{
	PyObject *builtins = _PyDict_GetItemIdWithError(globals, &PyId___builtins__);
	if (builtins) {
    if (PyModule_Check(builtins)) {
      assert(false);
    }
    return builtins;
	}
	if (PyErr_Occurred()) {
		return NULL;
	}

	return _PyEval_GetBuiltins(tstate);
}


