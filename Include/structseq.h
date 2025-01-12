#pragma once

#include "tupleobject.h"
#include "typeslots.h"

typedef struct PyStructSequence_Field {
  const char *name;
  const char *doc;
} PyStructSequence_Field;

typedef struct PyStructSequence_Desc {
  const char *name;
  const char *doc;
  struct PyStructSequence_Field *fields;
  int n_in_sequence;
} PyStructSequence_Desc;

typedef PyTupleObject PyStructSequence;

static const char visible_length_key[] = "n_sequence_fields";
static const char real_length_key[] = "n_fields";
static const char unnamed_fields_key[] = "n_unnamed_fields";
static const char match_args_key[] = "__match_args__";

const char * const PyStructSequence_UnnamedField = "unnamed field";

_Py_IDENTIFIER(n_sequence_fields);
_Py_IDENTIFIER(n_fields);
_Py_IDENTIFIER(n_unnamed_fields);

static Py_ssize_t
get_type_attr_as_size(PyTypeObject *tp, _Py_Identifier *id) {
  PyObject *name = _PyUnicode_FromId(id);
  if (name == NULL) {
    return -1;
  }
  PyObject *v = PyDict_GetItemWithError(tp->tp_dict, name);
  if (v == NULL && !PyErr_Occurred()) {
    fail(0);
  }
  return PyLong_AsSsize_t(v);
}

static PyObject *
structseq_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
  fail(0);
}

static int
structseq_traverse(PyStructSequence *obj, visitproc visit, void *arg) {
  fail(0);
}

#define REAL_SIZE_TP(tp) get_type_attr_as_size(tp, &PyId_n_fields)
#define REAL_SIZE(op) REAL_SIZE_TP(Py_TYPE(op))
static void structseq_dealloc(PyStructSequence *obj) {
  Py_ssize_t i, size;
  PyTypeObject *tp;

  tp = (PyTypeObject *) Py_TYPE(obj);
  size = REAL_SIZE(obj);
  for (i = 0; i < size; ++i) {
    Py_XDECREF(obj->ob_item[i]);
  }
  PyObject_GC_Del(obj);
  if (_PyType_HasFeature(tp, Py_TPFLAGS_HEAPTYPE)) {
    Py_DECREF(tp);
  }
}

static PyObject *
structseq_repr(PyStructSequence *obj) {
  PyTypeObject *typ = Py_TYPE(obj);
  _PyUnicodeWriter writer;

  // Write "typename("
  PyObject *type_name = PyUnicode_DecodeUTF8(typ->tp_name,
      strlen(typ->tp_name),
      NULL);
  if (type_name == NULL) {
    return NULL;
  }

  _PyUnicodeWriter_Init(&writer);
  fail(0);
}

static PyMethodDef structseq_methods[] = {
  {NULL, NULL},
};


static Py_ssize_t
count_members(PyStructSequence_Desc *desc, Py_ssize_t *n_unnamed_members) {
  Py_ssize_t i;
  *n_unnamed_members = 0;
  for (i = 0; desc->fields[i].name != NULL; ++i) {
    if (desc->fields[i].name == PyStructSequence_UnnamedField) {
      (*n_unnamed_members)++;
    }
  }
  return i;
}

static void
initialize_members(PyStructSequence_Desc *desc, PyMemberDef *members,
    Py_ssize_t n_members) {
  Py_ssize_t i, k;

  for (i = k = 0; i < n_members; ++i) {
    if (desc->fields[i].name == PyStructSequence_UnnamedField) {
      continue;
    }

    members[k].name = desc->fields[i].name;
    members[k].type = T_OBJECT;
    members[k].offset = offsetof(PyStructSequence, ob_item)
      + i * sizeof(PyObject *);
    members[k].flags = READONLY;
    members[k].doc = desc->fields[i].doc;
    k++;
  }
  members[k].name = NULL;
}

static int
initialize_structseq_dict(PyStructSequence_Desc *desc, PyObject *dict,
    Py_ssize_t n_members, Py_ssize_t n_unnamed_members) {
  PyObject *v;

#define SET_DICT_FROM_SIZE(key, value) \
  do { \
    v = PyLong_FromSsize_t(value); \
    if (v == NULL) { \
      return -1; \
    } \
    if (PyDict_SetItemString(dict, key, v) < 0) { \
      Py_DECREF(v); \
      return -1; \
    } \
    Py_DECREF(v); \
  } while (0)

  SET_DICT_FROM_SIZE(visible_length_key, desc->n_in_sequence);
  SET_DICT_FROM_SIZE(real_length_key, n_members);
  SET_DICT_FROM_SIZE(unnamed_fields_key, n_unnamed_members);

  // TODO: some namedtuple fields are set here
  return 0;
}

int _PyStructSequence_InitType(PyTypeObject *type, PyStructSequence_Desc *desc, unsigned long tp_flags) {
  PyMemberDef *members;
  Py_ssize_t n_members, n_unnamed_members;

  if (Py_REFCNT(type) != 0) {
    fail(0);
  }

  type->tp_name = desc->name;
  type->tp_basicsize = sizeof(PyStructSequence) - sizeof(PyObject *);
  type->tp_itemsize = sizeof(PyObject *);
  type->tp_dealloc = (destructor) structseq_dealloc;
  type->tp_repr = (reprfunc) structseq_repr;
  type->tp_doc = desc->doc;
  type->tp_base = &PyTuple_Type;
  type->tp_methods = structseq_methods;
  type->tp_new = structseq_new;
  type->tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | tp_flags;
  type->tp_traverse = (traverseproc) structseq_traverse;

  n_members = count_members(desc, &n_unnamed_members);
  members = PyMem_NEW(PyMemberDef, n_members - n_unnamed_members + 1);
  if (members == NULL) {
    fail(0);
  }

  initialize_members(desc, members, n_members);
  type->tp_members = members;

  if (PyType_Ready(type) < 0) {
    fail(0);
  }
  Py_INCREF(type);

  if (initialize_structseq_dict(
      desc, type->tp_dict, n_members, n_unnamed_members) < 0) {
    fail(0);
  }

  return 0;
}

#define VISIBLE_SIZE_TP(tp) get_type_attr_as_size(tp, &PyId_n_sequence_fields)
#define REAL_SIZE_TP(tp) get_type_attr_as_size(tp, &PyId_n_fields)

PyObject *
PyStructSequence_New(PyTypeObject *type) {
  PyStructSequence *obj;
  Py_ssize_t size = REAL_SIZE_TP(type), i;
  if (size < 0) {
    return NULL;
  }
  Py_ssize_t vsize = VISIBLE_SIZE_TP(type);
  if (vsize < 0) {
    return NULL;
  }

  obj = PyObject_GC_NewVar(PyStructSequence, type, size);
  if (obj == NULL) {
    return NULL;
  }
  Py_SET_SIZE(obj, vsize);
  for (i = 0; i < size; i++) {
    obj->ob_item[i] = NULL;
  }
  return (PyObject *) obj;
}

#define PyStructSequence_SET_ITEM(op, i, v) PyTuple_SET_ITEM(op, i, v)
#define PyStructSequence_GET_ITEM(op, i) PyTuple_GET_ITEM(op, i)

PyTypeObject *
PyStructSequence_NewType(PyStructSequence_Desc *desc) {
  PyMemberDef *members;
  PyTypeObject *type;
  PyType_Slot slots[8];
  PyType_Spec spec;
  Py_ssize_t n_members, n_unnamed_members;

  n_members = count_members(desc, &n_unnamed_members);
  members = PyMem_NEW(PyMemberDef, n_members - n_unnamed_members + 1);
  if (members == NULL) {
    fail(0);
  }
  initialize_members(desc, members, n_members);

  slots[0] = (PyType_Slot) {Py_tp_dealloc, (destructor) structseq_dealloc};
  slots[1] = (PyType_Slot) {Py_tp_repr, (reprfunc) structseq_repr};
  slots[2] = (PyType_Slot) {Py_tp_doc, (void *) desc->doc};
  slots[3] = (PyType_Slot) {Py_tp_methods, structseq_methods };
  slots[4] = (PyType_Slot) {Py_tp_new, structseq_new };
  slots[5] = (PyType_Slot) {Py_tp_members, members };
  slots[6] = (PyType_Slot) {Py_tp_traverse, (traverseproc) structseq_traverse };
  slots[7] = (PyType_Slot) {0, 0 };

  spec.name = desc->name;
  spec.basicsize = sizeof(PyStructSequence) - sizeof(PyObject *);
  spec.itemsize = sizeof(PyObject *);
  spec.flags = 0;
  spec.slots = slots;

  type = (PyTypeObject *) PyType_FromSpecWithBases(&spec, (PyObject *) &PyTuple_Type);
  PyMem_Free(members);
  if (type == NULL) {
    return NULL;
  }

  if (initialize_structseq_dict(
      desc, type->tp_dict, n_members, n_unnamed_members) < 0) {
    Py_DECREF(type);
    return NULL;
  }

  return type;
}
