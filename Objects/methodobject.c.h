#pragma once

PyTypeObject PyCMethod_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "builtin_method",
  .tp_basicsize = sizeof(PyCMethodObject),
  .tp_base = &PyCFunction_Type,
};

static PyObject *meth_repr(PyCFunctionObject *m) {
  if (m->m_self == NULL || PyModule_Check(m->m_self))
    return PyUnicode_FromFormat("<built-in function %s>",
        m->m_ml->ml_name);
  return PyUnicode_FromFormat("<built-in method %s of %s object at %p>",
    m->m_ml->ml_name,
    Py_TYPE(m->m_self)->tp_name,
    m->m_self);
}


