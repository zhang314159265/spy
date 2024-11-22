#pragma once

#define _PyGenObject_HEAD(prefix) \
  PyObject_HEAD \
  PyFrameObject *prefix ## _frame; \
  PyObject *prefix ## _code; \
  PyObject *prefix ## _weakreflist; \
  PyObject *prefix ## _name; \
  PyObject *prefix ## _qualname;

typedef struct {
  _PyGenObject_HEAD(gi);
} PyGenObject;

typedef struct {
  _PyGenObject_HEAD(ag)
} PyAsyncGenObject;

#define PyGen_CheckExact(op) Py_IS_TYPE(op, &PyGen_Type)
#define PyAsyncGen_CheckExact(op) Py_IS_TYPE(op, &PyAsyncGen_Type)

static PyObject *gen_iternext(PyGenObject *gen);

static void gen_dealloc(PyGenObject *gen);
void _PyGen_Finalize(PyObject *self);

PyTypeObject PyGen_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "generator",
  .tp_basicsize = sizeof(PyGenObject),
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
  .tp_iter = PyObject_SelfIter,
  .tp_iternext = (iternextfunc) gen_iternext,
  .tp_dealloc = (destructor) gen_dealloc,
  .tp_finalize = _PyGen_Finalize,
};

PyTypeObject PyAsyncGen_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "async_generator",
  .tp_basicsize = sizeof(PyAsyncGenObject),
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
};

static PyObject *
gen_new_with_qualname(PyTypeObject *type, PyFrameObject *f,
    PyObject *name, PyObject *qualname) {
  PyGenObject *gen = PyObject_GC_New(PyGenObject, type);
  if (gen == NULL) {
    Py_DECREF(f);
    return NULL;
  }
  gen->gi_frame = f;
  f->f_gen = (PyObject *) gen;
  Py_INCREF(f->f_code);
  gen->gi_code = (PyObject *) (f->f_code);
  gen->gi_weakreflist = NULL;

  if (name != NULL)
    gen->gi_name = name;
  else
    gen->gi_name = ((PyCodeObject *)gen->gi_code)->co_name;
  Py_INCREF(gen->gi_name);
  if (qualname != NULL)
    gen->gi_qualname = qualname;
  else
    gen->gi_qualname = gen->gi_name;
  Py_INCREF(gen->gi_qualname);
  _PyObject_GC_TRACK(gen);
  return (PyObject *) gen;
}

PyObject *PyGen_NewWithQualName(PyFrameObject *f,
    PyObject *name, PyObject *qualname) {
  return gen_new_with_qualname(&PyGen_Type, f, name, qualname);
}

static inline PyObject *_PyEval_EvalFrame(PyThreadState *tstate, PyFrameObject *f, int throwflag);

static PySendResult
gen_send_ex2(PyGenObject *gen, PyObject *arg, PyObject **presult,
    int exc, int closing) {
  PyThreadState *tstate = _PyThreadState_GET();
  PyFrameObject *f = gen->gi_frame;
  PyObject *result;

  *presult = NULL;
  if (f != NULL && f->f_lasti < 0 && arg && arg != Py_None) {
    assert(false);
  }
  if (f != NULL && _PyFrame_IsExecuting(f)) {
    assert(false);
  }
  if (f == NULL || _PyFrameHasCompleted(f)) {
    assert(false);
  }
  assert(_PyFrame_IsRunnable(f));
  // printf("f->f_lasti is %d\n", f->f_lasti);
  assert(f->f_lasti >= 0 || ((unsigned char *) PyBytes_AS_STRING(f->f_code->co_code))[0] == GEN_START);

  // Push arg onto the frame's value stack
  result = arg ? arg : Py_None;
  Py_INCREF(result);
  gen->gi_frame->f_valuestack[gen->gi_frame->f_stackdepth] = result;
  gen->gi_frame->f_stackdepth++;

  Py_XINCREF(tstate->frame);
  assert(f->f_back == NULL);
  f->f_back = tstate->frame;

  // gen->gi_exc_state.previous_item = tstate->exc_info;
  // tstate->exc_info = &gen->gi_exc_state;

  if (exc) {
    assert(false);
  }

  result = _PyEval_EvalFrame(tstate, f, exc);

  assert(f->f_back == tstate->frame);
  Py_CLEAR(f->f_back);

  if (result) {
    if (!_PyFrameHasCompleted(f)) {
      *presult = result;
      return PYGEN_NEXT;
    }
    assert(result == Py_None || !PyAsyncGen_CheckExact(gen));
    if (result == Py_None && !PyAsyncGen_CheckExact(gen) && !arg) {
      Py_CLEAR(result);
    }
  } else {
    assert(false);
  }

  gen->gi_frame->f_gen = NULL;
  gen->gi_frame = NULL;
  Py_DECREF(f);

  *presult = result;
  return result ? PYGEN_RETURN : PYGEN_ERROR;
}

static PyObject *gen_iternext(PyGenObject *gen) {
  PyObject *result;
  assert(PyGen_CheckExact(gen));
  if (gen_send_ex2(gen, NULL, &result, 0, 0) == PYGEN_RETURN) {
    assert(false);
  }
  return result;
}

static void gen_dealloc(PyGenObject *gen) {
  PyObject *self = (PyObject *) gen;

  _PyObject_GC_UNTRACK(gen);

  if (gen->gi_weakreflist != NULL) {
    assert(false);
  }

  _PyObject_GC_TRACK(self);

  if (PyObject_CallFinalizerFromDealloc(self))
    return;

  _PyObject_GC_UNTRACK(self);
  if (PyAsyncGen_CheckExact(gen)) {
    assert(false);
  }
  if (gen->gi_frame != NULL) {
    gen->gi_frame->f_gen = NULL;
    Py_CLEAR(gen->gi_frame);
  }
  if (((PyCodeObject *)gen->gi_code)->co_flags & CO_COROUTINE) {
    assert(false);
  }
  Py_CLEAR(gen->gi_code);
  Py_CLEAR(gen->gi_name);
  Py_CLEAR(gen->gi_qualname);
  PyObject_GC_Del(gen);
}

void _PyGen_Finalize(PyObject *self) {
  PyGenObject *gen = (PyGenObject *) self;
  PyObject *res = NULL;

  if (gen->gi_frame == NULL || _PyFrameHasCompleted(gen->gi_frame)) {
    return;
  }
  assert(false);
}
