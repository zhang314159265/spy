#pragma once


PyObject *PyMember_GetOne(const char *obj_addr, PyMemberDef *l) {
  PyObject *v;

  const char *addr = obj_addr + l->offset;
  switch (l->type) {
  case T_OBJECT:
    v = *(PyObject **) addr;
    if (v == NULL)
      v = Py_None;
    Py_INCREF(v);
    break;
  case T_UINT:
    v = PyLong_FromUnsignedLong(*(unsigned int *) addr);
    break;
  default:
    fprintf(stderr, "PyMember_GetOne type is %d\n", l->type);
    assert(false);
  }
  return v;
}

int PyMember_SetOne(char *addr, PyMemberDef *l, PyObject *v) {
  PyObject *oldv;

  addr += l->offset;

  if ((l->flags & READONLY)) {
    assert(false);
  }
  if (v == NULL) {
    assert(false);
  }
  switch (l->type) {
  case T_OBJECT:
    Py_XINCREF(v);
    oldv = *(PyObject **) addr;
    *(PyObject **)addr = v;
    Py_XDECREF(oldv);
    break;
  default:
    fprintf(stderr, "type is %d\n", l->type);
    assert(false);
  }
  return 0;
}
