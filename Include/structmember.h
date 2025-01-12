#pragma once

typedef struct PyMemberDef {
  const char *name;
  int type;
  Py_ssize_t offset;
  int flags;
  const char *doc;
} PyMemberDef;

#define T_OBJECT 6
#define T_UINT 11

#define READONLY 1
#define READ_RESTRICTED 2
#define PY_AUDIT_READ READ_RESTRICTED

PyObject *PyMember_GetOne(const char *obj_addr, PyMemberDef *l);
int PyMember_SetOne(char *addr, PyMemberDef *l, PyObject *v);
