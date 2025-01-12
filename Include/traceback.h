#pragma once

extern PyTypeObject PyTraceBack_Type;

#define PyTraceBack_Check(v) Py_IS_TYPE(v, &PyTraceBack_Type)

typedef struct _traceback {
  PyObject_HEAD
  struct _traceback *tb_next;
  PyFrameObject *tb_frame;
  int tb_lasti;
  int tb_lineno;
} PyTracebackObject;



static PyObject * tb_create_raw(PyTracebackObject *next, PyFrameObject *frame, int lasti, int lineno);

PyObject *
_PyTraceBack_FromFrame(PyObject *tb_next, PyFrameObject *frame) {
  assert(tb_next == NULL || PyTraceBack_Check(tb_next));
  assert(frame != NULL);

  return tb_create_raw((PyTracebackObject *) tb_next, frame, frame->f_lasti * sizeof(_Py_CODEUNIT), PyFrame_GetLineNumber(frame));
}

int PyTraceBack_Here(PyFrameObject *frame) {
  PyObject *exc, *val, *tb, *newtb;
  PyErr_Fetch(&exc, &val, &tb);
  newtb = _PyTraceBack_FromFrame(tb, frame);
  if (newtb == NULL) {
    fail(0);
  }
  PyErr_Restore(exc, val, newtb);
  Py_XDECREF(tb);
  return 0;
}

static void
tb_dealloc(PyTracebackObject *tb) {
  Py_XDECREF(tb->tb_next);
  Py_XDECREF(tb->tb_frame);
  PyObject_GC_Del(tb);
}

static int
tb_clear(PyTracebackObject *tb) {
  fail(0);
}

static PyObject *
tb_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
  fail(0);
}

PyTypeObject PyTraceBack_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "traceback",
  .tp_basicsize = sizeof(PyTracebackObject),
  .tp_dealloc = (destructor) tb_dealloc,
  .tp_getattro = PyObject_GenericGetAttr,
  .tp_clear = (inquiry) tb_clear,
  .tp_new = tb_new,
};

static PyObject * tb_create_raw(PyTracebackObject *next, PyFrameObject *frame, int lasti, int lineno) {
  PyTracebackObject *tb;
  if ((next != NULL && !PyTraceBack_Check(next)) ||
      frame == NULL || !PyFrame_Check(frame)) {
    fail(0);
  }
  tb = PyObject_GC_New(PyTracebackObject, &PyTraceBack_Type);
  if (tb != NULL) {
    Py_XINCREF(next);
    tb->tb_next = next;
    Py_XINCREF(frame);
    tb->tb_frame = frame;
    tb->tb_lasti = lasti;
    tb->tb_lineno = lineno;
  }
  return (PyObject *) tb;
}


