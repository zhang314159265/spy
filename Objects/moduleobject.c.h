#pragma once

static PyMemberDef module_members[] = {
  {"__dict__", T_OBJECT, offsetof(PyModuleObject, md_dict), READONLY},
  {0},
};

static PyObject *
module_repr(PyModuleObject *m) {
  #if 0
  PyInterpreterState *interp = _PyInterpreterState_GET();
  
  return PyObject_CallMethod(interp->importlib, "_module_repr", "O", m);
  #endif
  // TODO follow cpy
  if (m && m->md_name && PyUnicode_Check(m->md_name)) {
    return m->md_name;
  } else {
    return unicode_get_empty();
  }
}

PyTypeObject PyModule_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "module",
	.tp_basicsize = sizeof(PyModuleObject),
	.tp_setattro = PyObject_GenericSetAttr,
	.tp_dictoffset = offsetof(PyModuleObject, md_dict),
  .tp_weaklistoffset = offsetof(PyModuleObject, md_weaklist),
  .tp_new = PyType_GenericNew,
  .tp_members = module_members,
  .tp_repr = (reprfunc) module_repr,
};

PyObject *
PyModule_GetDict(PyObject *m) {
	if (!PyModule_Check(m)) {
		assert(false);
	}
	return _PyModule_GetDict(m);
}

PyObject *PyModule_FromDefAndSpec2(PyModuleDef *def,
    PyObject *spec,
    int module_api_version) {
  PyModuleDef_Slot *cur_slot;
  PyObject *(*create)(PyObject *, PyModuleDef*) = NULL;
  PyObject *nameobj;
  PyObject *m = NULL;
  int has_execution_slots = 0;
  const char *name;
  int ret;

  PyModuleDef_Init(def);

  nameobj = PyObject_GetAttrString(spec, "name");
  if (nameobj == NULL) {
    return NULL;
  }
  name = PyUnicode_AsUTF8(nameobj);
  if (name == NULL) {
    fail(0);
  }

  if (!check_api_version(name, module_api_version)) {
    fail(0);
  }

  if (def->m_size < 0) {
    fail(0);
  }

  for (cur_slot = def->m_slots; cur_slot && cur_slot->slot; cur_slot++) {
    if (cur_slot->slot == Py_mod_create) {
      if (create) {
        fail(0);
      }
      create = cur_slot->value;
    } else if (cur_slot->slot < 0 || cur_slot->slot > _Py_mod_LAST_SLOT) {
      fail(0);
    } else {
      has_execution_slots = 1;
    }
  }

  if (create) {
    fail(0);
  } else {
    m = PyModule_NewObject(nameobj);
    if (m == NULL) {
      fail(0);
    }
  }

  if (PyModule_Check(m)) {
    ((PyModuleObject *)m)->md_state = NULL;
    ((PyModuleObject *)m)->md_def = def;
  } else {
    fail(0);
  }

  if (def->m_methods != NULL) {
    ret = _add_methods_to_object(m, nameobj, def->m_methods);
    if (ret != 0) {
      fail(0);
    }
  }

  Py_DECREF(nameobj);
  return m;
}


