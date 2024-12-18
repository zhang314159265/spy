#pragma once

typedef struct {
  PyObject_HEAD
  PyObject *im_func;
  PyObject *im_self;
  PyObject *im_weakreflist;
  vectorcallfunc vectorcall;
} PyMethodObject;

PyObject *PyMethod_New(PyObject *func, PyObject *self);

#define PyMethod_GET_FUNCTION(meth) \
  (((PyMethodObject *) meth)->im_func)

#define PyMethod_GET_SELF(meth) \
  (((PyMethodObject *) meth)->im_self)
