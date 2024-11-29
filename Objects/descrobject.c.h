#pragma once

#include "modsupport.h"

static int
property_init_impl(propertyobject *self, PyObject *fget, PyObject *fset,
    PyObject *fdel, PyObject *doc) {
  if (fget == Py_None)
    fget = NULL;
  if (fset == Py_None)
    fset = NULL;
  if (fdel == Py_None)
    fdel = NULL;

  Py_XINCREF(fget);
  Py_XINCREF(fset);
  Py_XINCREF(fdel);
  Py_XINCREF(doc);

  Py_XSETREF(self->prop_get, fget);
  Py_XSETREF(self->prop_set, fset);
  Py_XSETREF(self->prop_del, fdel);
  Py_XSETREF(self->prop_doc, doc);
  Py_XSETREF(self->prop_name, NULL);

  self->getter_doc = 0;

  if ((doc == NULL || doc == Py_None) && fget != NULL) {
    _Py_IDENTIFIER(__doc__);
    PyObject *get_doc;
    int rc = _PyObject_LookupAttrId(fget, &PyId___doc__, &get_doc);
    if (rc <= 0) {
      return rc;
    }
    assert(false);
  }

  assert(false);
}

static int property_init(PyObject *self, PyObject *args, PyObject *kwargs) {
  int return_value = -1;
  static const char * const _keywords[] = {"fget", "fset", "fdel", "doc", NULL};
  static _PyArg_Parser _parser = {NULL, _keywords, "property", 0};
  PyObject *argsbuf[4];
  PyObject *const *fastargs;
  Py_ssize_t nargs = PyTuple_GET_SIZE(args);
  Py_ssize_t noptargs = nargs + (kwargs ? PyDict_GET_SIZE(kwargs) : 0);
  PyObject *fget = NULL;
  PyObject *fset = NULL;
  PyObject *fdel = NULL;
  PyObject *doc = NULL;


  fastargs = _PyArg_UnpackKeywords(_PyTuple_CAST(args)->ob_item, nargs, kwargs, NULL, &_parser, 0, 4, 0, argsbuf);
  if (!fastargs) {
    assert(false);
  }
  if (!noptargs) {
    assert(false);
  }
  if (fastargs[0]) {
    fget = fastargs[0];
    if (!--noptargs) {
      goto skip_optional_pos;
    }
  }
  if (fastargs[1]) {
    fset = fastargs[1];
    if (!--noptargs) {
      goto skip_optional_pos;
    }
  }
  if (fastargs[2]) {
    fdel = fastargs[2];
    if (!--noptargs) {
      goto skip_optional_pos;
    }
  }
  doc = fastargs[3];
 skip_optional_pos:
  return_value = property_init_impl((propertyobject *) self, fget, fset, fdel, doc);

  return return_value;
}
