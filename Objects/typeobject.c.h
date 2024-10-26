#pragma once

#include "descrobject.h"

static int
type_is_subtype_base_chain(PyTypeObject *a, PyTypeObject *b) {
  do {
    if (a == b)
      return 1;
    a = a->tp_base;
  } while (a != NULL);

  return (b == &PyBaseObject_Type);
}

// Generic type check
// defined in cpy/Objects/typeobject.c
int PyType_IsSubtype(PyTypeObject *a, PyTypeObject *b) {
  PyObject *mro;

  mro = a->tp_mro;
	if (mro != NULL) {
		Py_ssize_t i, n;
		assert(PyTuple_Check(mro));
		n = PyTuple_GET_SIZE(mro);
		for (i = 0; i < n; i++) {
			if (PyTuple_GET_ITEM(mro, i) == (PyObject *) b)
				return 1;
		}
		return 0;
	}
	else {
		// printf("type name is a %s b %s\n", a->tp_name, b->tp_name);
    return type_is_subtype_base_chain(a, b);
	}
}

static PyObject *
type_call(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  PyObject *obj;
  PyThreadState *tstate = _PyThreadState_GET();

  if (type == &PyType_Type) {
    assert(args != NULL && PyTuple_Check(args));
    assert(kwds == NULL || PyDict_Check(kwds));
    Py_ssize_t nargs = PyTuple_GET_SIZE(args);

    if (nargs == 1 && (kwds == NULL || !PyDict_GET_SIZE(kwds))) {
      assert(false);
    }
    if (nargs != 3) {
      assert(false);
    }
  }

  if (type->tp_new == NULL) {
    printf("type name is %s\n", type->tp_name);
    assert(false);
  }

  obj = type->tp_new(type, args, kwds);
  obj = _Py_CheckFunctionResult(tstate, (PyObject *) type, obj, NULL);
  if (obj == NULL)
    return NULL;

  if (!PyType_IsSubtype(Py_TYPE(obj), type))
    return obj;

  type = Py_TYPE(obj);
  if (type->tp_init != NULL) {
    int res = type->tp_init(obj, args, kwds);
    if (res < 0) {
      assert(false);
    } else {
      assert(!_PyErr_Occurred(tstate));
    }
  }
  return obj;
}

static PyObject *
type_getattro(PyTypeObject *type, PyObject *name) {
  PyTypeObject *metatype = Py_TYPE(type); 
  PyObject *meta_attribute, *attribute;
  descrgetfunc meta_get;
  PyObject *res;

  if (!PyUnicode_Check(name)) {
    assert(false);
  }

  if (!_PyType_IsReady(type)) {
    if (PyType_Ready(type) < 0)
      return NULL;
  }

  meta_get = NULL;

  meta_attribute = _PyType_Lookup(metatype, name);
  if (meta_attribute != NULL) {
    Py_INCREF(meta_attribute);
    meta_get = Py_TYPE(meta_attribute)->tp_descr_get;
    printf("meta_attribute class name %s\n", Py_TYPE(meta_attribute)->tp_name);
    if (meta_get != NULL && PyDescr_IsData(meta_attribute)) {
      assert(false);
    }
  }
  attribute = _PyType_Lookup(type, name);
  if (attribute != NULL) {
    Py_INCREF(attribute);
    descrgetfunc local_get = Py_TYPE(attribute)->tp_descr_get;

    Py_XDECREF(meta_attribute);

    if (local_get != NULL) {
      res = local_get(attribute, (PyObject *) NULL,
          (PyObject *) type);
      Py_DECREF(attribute);
      return res;
    }
    return attribute;
  }
  assert(false);
}

static PyObject *
type_prepare(PyObject *self, PyObject *const *args, Py_ssize_t nargs,
    PyObject *kwnames) {
  return PyDict_New();
}

static PyMethodDef type_methods[] = {
  {"__prepare__", (PyCFunction)(void(*)(void)) type_prepare,
    METH_FASTCALL | METH_KEYWORDS | METH_CLASS,
    PyDoc_STR("") },
  {0},
};

static PyObject *type_vectorcall(PyObject *metatype, PyObject *const *args, size_t nargsf, PyObject *kwnames);
static PyObject *type_new(PyTypeObject *metatype, PyObject *args, PyObject *kwds);
static int type_init(PyObject *cls, PyObject *args, PyObject *kwds);

PyTypeObject PyType_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "type",
  .tp_basicsize = sizeof(PyHeapTypeObject),
  .tp_flags = Py_TPFLAGS_TYPE_SUBCLASS
    | Py_TPFLAGS_HAVE_VECTORCALL,
	.tp_call = (ternaryfunc) type_call,
	.tp_vectorcall_offset = offsetof(PyTypeObject, tp_vectorcall),
  .tp_getattr = 0,
  .tp_getattro = (getattrofunc) type_getattro,
  .tp_methods = type_methods,
  .tp_vectorcall = type_vectorcall,
  .tp_new = type_new,
  .tp_init = type_init,
};

typedef struct {
  PyObject_HEAD
  PyTypeObject *type;
  PyObject *obj;
  PyTypeObject *obj_type;
} superobject;

PyObject *PyType_GenericNew(PyTypeObject *type, PyObject *args, PyObject *kwds);

static int super_init(PyObject *self, PyObject *args, PyObject *kwds);
static PyObject *super_getattro(PyObject *self, PyObject *name);

static void super_dealloc(PyObject *self);

PyTypeObject PySuper_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "super",
  .tp_basicsize = sizeof(superobject),
  .tp_itemsize = 0,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC |
      Py_TPFLAGS_BASETYPE,
  .tp_new = PyType_GenericNew,
  .tp_alloc = PyType_GenericAlloc,
  .tp_init = super_init,
  .tp_getattro = super_getattro,
  .tp_dealloc = super_dealloc,
  .tp_free = PyObject_GC_Del,
};

static int type_add_method(PyTypeObject *type, PyMethodDef *meth) {
	PyObject *descr;
	int isdescr = 1;

	if (meth->ml_flags & METH_CLASS) {
    if (meth->ml_flags & METH_STATIC) {
      assert(false);
    }
    descr = PyDescr_NewClassMethod(type, meth);
	} else if (meth->ml_flags & METH_STATIC) {
		assert(false);
	} else {
		descr = PyDescr_NewMethod(type, meth);
	}
	if (descr == NULL) {
		return -1;
	}

	PyObject *name;
	if (isdescr) {
		name = PyDescr_NAME(descr);
	} else {
		assert(false);
	}

	int err;
	if (!(meth->ml_flags & METH_COEXIST)) {
		err = PyDict_SetDefault(type->tp_dict, name, descr) == NULL;
	} else {
		assert(false);
	}
	if (!isdescr) {
		Py_DECREF(name);
	}
	Py_DECREF(descr);
	if (err) {
		return -1;
	}
	return 0;
}

static int type_add_methods(PyTypeObject *type) {
	PyMethodDef *meth = type->tp_methods;
	if (meth == NULL) {
		return 0;
	}
	for (; meth->ml_name != NULL; meth++) {
		if (type_add_method(type, meth) < 0) {
			return -1;
		}
	}
	return 0;
}

PyTypeObject *_PyType_CalculateMetaclass(PyTypeObject *metatype, PyObject *bases) {
  Py_ssize_t i, nbases;
  PyTypeObject *winner;

  nbases = PyTuple_GET_SIZE(bases);
  winner = metatype;
  for (i = 0; i < nbases; i++) {
    assert(false);
  }
  return winner;
}

PyObject *_PyObject_MakeTpCall(PyThreadState *tstate, PyObject *callable,
    PyObject *const *args, Py_ssize_t nargs,
    PyObject *keywords);

static PyObject *type_vectorcall(PyObject *metatype, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
  Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
  if (nargs == 1 && metatype == (PyObject *) &PyType_Type) {
    assert(false);
  }

  PyThreadState *tstate = PyThreadState_GET();
  return _PyObject_MakeTpCall(tstate, metatype, args, nargs, kwnames);
}

typedef struct {
  PyTypeObject *metatype;
  PyObject *args;
  PyObject *kwds;
  PyObject *orig_dict;
  PyObject *name;
  PyObject *bases;
  PyTypeObject *base;
  PyObject *slots;
  Py_ssize_t nslot;
  int add_dict;
  int add_weak;
  int may_add_dict;
  int may_add_weak;
} type_new_ctx;

static int
type_new_get_bases(type_new_ctx *ctx, PyObject **type) {
  Py_ssize_t nbases = PyTuple_GET_SIZE(ctx->bases);
  if (nbases == 0) {
    ctx->base = &PyBaseObject_Type;
    PyObject *new_bases = PyTuple_Pack(1, ctx->base);
    if (new_bases == NULL) {
      return -1;
    }
    ctx->bases = new_bases;
    return 0;
  }
  assert(false);
}

static int
type_new_get_slots(type_new_ctx *ctx, PyObject *dict) {
  _Py_IDENTIFIER(__slots__);
  PyObject *slots = _PyDict_GetItemIdWithError(dict, &PyId___slots__);
  if (slots == NULL) {
    if (PyErr_Occurred()) {
      return -1;
    }
    ctx->slots = NULL;
    ctx->nslot = 0;
    return 0;
  }
  assert(false);
}

static Py_ssize_t
type_new_slots(type_new_ctx *ctx, PyObject *dict) {
  ctx->add_dict = 0;
  ctx->add_weak = 0;
  ctx->may_add_dict = (ctx->base->tp_dictoffset == 0);
  ctx->may_add_weak = (ctx->base->tp_weaklistoffset == 0
      && ctx->base->tp_itemsize == 0);

  if (ctx->slots == NULL) {
    if (ctx->may_add_dict) {
      ctx->add_dict++;
    }
    if (ctx->may_add_weak) {
      ctx->add_weak++;
    }
  } else {
    assert(false);
  }
  return 0;
}

static void
subtype_dealloc(PyObject *self) {
  assert(false);
}

static int subtype_traverse(PyObject *self, visitproc visit, void *arg) {
  assert(false);
}

static int
subtype_clear(PyObject *self) {
  assert(false);
}

static PyTypeObject *
type_new_alloc(type_new_ctx *ctx) {
  PyTypeObject *metatype = ctx->metatype;
  PyTypeObject *type;
  
  type = (PyTypeObject *) metatype->tp_alloc(metatype, ctx->nslot);
  if (type == NULL) {
    return NULL;
  }
  PyHeapTypeObject *et = (PyHeapTypeObject *) type;

  type->tp_flags = (Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE |
      Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC);

  // Initialize essential fields
  type->tp_as_async = &et->as_async;
  type->tp_as_number = &et->as_number;
  type->tp_as_sequence = &et->as_sequence;
  type->tp_as_mapping = &et->as_mapping;
  type->tp_as_buffer = &et->as_buffer;

  type->tp_bases = Py_NewRef(ctx->bases);
  type->tp_base = (PyTypeObject *) Py_NewRef(ctx->base);

  type->tp_dealloc = subtype_dealloc;
  // Always override allocation strategy to use regular heap
  type->tp_alloc = PyType_GenericAlloc;
  type->tp_free = PyObject_GC_Del;

  type->tp_traverse = subtype_traverse;
  type->tp_clear = subtype_clear;

  et->ht_name = Py_NewRef(ctx->name);
  et->ht_module = NULL;

  return type;
}

static PyTypeObject *
type_new_init(type_new_ctx *ctx) {
  PyObject *dict = PyDict_Copy(ctx->orig_dict);
  if (dict == NULL) {
    assert(false);
  }

  if (type_new_get_slots(ctx, dict) < 0) {
    assert(false);
  }
  assert(!PyErr_Occurred());

  if (type_new_slots(ctx, dict) < 0) {
    assert(false);
  }

  PyTypeObject *type = type_new_alloc(ctx);
  if (type == NULL) {
    assert(false);
  }

  type->tp_dict = dict;

  PyHeapTypeObject *et = (PyHeapTypeObject *) type;
  et->ht_slots = ctx->slots;
  ctx->slots = NULL;

  return type;
}

static int
type_new_set_name(const type_new_ctx *ctx, PyTypeObject *type) {
  Py_ssize_t name_size;
  type->tp_name = PyUnicode_AsUTF8AndSize(ctx->name, &name_size);
  if (!type->tp_name) {
    return -1;
  }
  if (strlen(type->tp_name) != (size_t) name_size) {
    assert(false);
  }
  return 0;
}

// set __module__ in the dict
static int
type_new_set_module(PyTypeObject *type) {
  _Py_IDENTIFIER(__module__);

  int r = _PyDict_ContainsId(type->tp_dict, &PyId___module__);
  if (r < 0) {
    return -1;
  }
  if (r > 0) {
    return 0;
  }

  assert(false);
}

static int
type_new_set_ht_name(PyTypeObject *type) {
  _Py_IDENTIFIER(__qualname__);

  PyHeapTypeObject *et = (PyHeapTypeObject *) type;
  PyObject *qualname = _PyDict_GetItemIdWithError(type->tp_dict,
      &PyId___qualname__);
  if (qualname != NULL) {
    if (!PyUnicode_Check(qualname)) {
      assert(false);
    }
    et->ht_qualname = Py_NewRef(qualname);
    if (_PyDict_DelItemId(type->tp_dict, &PyId___qualname__) < 0) {
      return -1;
    }
  } else {
    assert(false);
  }
  return 0;
}

static int
type_new_set_doc(PyTypeObject *type) {
  _Py_IDENTIFIER(__doc__);

  PyObject *doc = _PyDict_GetItemIdWithError(type->tp_dict, &PyId___doc__);
  if (doc == NULL) {
    if (PyErr_Occurred()) {
      return -1;
    }
    return 0;
  }
  assert(false);
}

static int
type_new_staticmethod(PyTypeObject *type, _Py_Identifier *attr_id) {
  PyObject *func = _PyDict_GetItemIdWithError(type->tp_dict, attr_id);
  if (func == NULL) {
    if (PyErr_Occurred()) {
      return -1;
    }
    return 0;
  }
  assert(false);
}

static int
type_new_classmethod(PyTypeObject *type, _Py_Identifier *attr_id) {
  PyObject *func = _PyDict_GetItemIdWithError(type->tp_dict, attr_id);
  if (func == NULL) {
    if (PyErr_Occurred()) {
      return -1;
    }
    return 0;
  }
  assert(false);
}

static int
type_new_descriptors(const type_new_ctx *ctx, PyTypeObject *type) {
  PyHeapTypeObject *et = (PyHeapTypeObject *) type;
  Py_ssize_t slotoffset = ctx->base->tp_basicsize;

  if (et->ht_slots != NULL) {
    assert(false);
  }

  if (ctx->add_dict) {
    if (ctx->base->tp_itemsize) {
      assert(false);
    } else {
      type->tp_dictoffset = slotoffset;
    }
    slotoffset += sizeof(PyObject *);
  }

  if (ctx->add_weak) {
    assert(!ctx->base->tp_itemsize);
    type->tp_weaklistoffset = slotoffset;
    slotoffset += sizeof(PyObject *);
  }

  type->tp_basicsize = slotoffset;
  type->tp_itemsize = ctx->base->tp_itemsize;
  type->tp_members = PyHeapType_GET_MEMBERS(et);
  return 0;
}

static PyObject *
subtype_dict(PyObject *obj, void *context) {
  assert(false);
}

static int
subtype_setdict(PyObject *obj, PyObject *value, void *context) {
  assert(false);
}

static PyObject *
subtype_getweakref(PyObject *obj, void *context) {
  assert(false);
}

static PyGetSetDef subtype_getsets_full[] = {
  {"__dict__", subtype_dict, subtype_setdict,
    PyDoc_STR("")},
  {"__weakref__", subtype_getweakref, NULL,
    PyDoc_STR("")},
  {0}
};

static void
type_new_set_slots(const type_new_ctx *ctx, PyTypeObject *type) {
  if (type->tp_weaklistoffset && type->tp_dictoffset) {
    type->tp_getset = subtype_getsets_full;
  } else {
    assert(false);
  }

  // special case some slots
  if (type->tp_dictoffset != 0 || ctx->nslot > 0) {
    PyTypeObject *base = ctx->base;
    if (base->tp_getattr == NULL && base->tp_getattro == NULL) {
      type->tp_getattro = PyObject_GenericGetAttr;
    }
    if (base->tp_setattr == NULL && base->tp_setattro == NULL) {
      type->tp_setattro = PyObject_GenericSetAttr;
    }
  }
}

// store type in class' cell if one is supplied
static int
type_new_set_classcell(PyTypeObject *type) {
  _Py_IDENTIFIER(__classcell__);
  PyObject *cell = _PyDict_GetItemIdWithError(type->tp_dict,
      &PyId___classcell__);
  if (cell == NULL) {
    if (PyErr_Occurred()) {
      return -1;
    } 
    return 0;
  }
  assert(false);
}

static int
type_new_set_attrs(const type_new_ctx *ctx, PyTypeObject *type) {
  _Py_IDENTIFIER(__new__);
  _Py_IDENTIFIER(__init_subclass__);
  _Py_IDENTIFIER(__class_getitem__);

  if (type_new_set_name(ctx, type) < 0) {
    return -1;
  }

  if (type_new_set_module(type) < 0) {
    return -1;
  }

  if (type_new_set_ht_name(type) < 0) {
    return -1;
  }

  if (type_new_set_doc(type) < 0) {
    return -1;
  }

  if (type_new_staticmethod(type, &PyId___new__) < 0) {
    return -1;
  }

  if (type_new_classmethod(type, &PyId___init_subclass__) < 0) {
    return -1;
  }
  if (type_new_classmethod(type, &PyId___class_getitem__) < 0) {
    return -1;
  }

  if (type_new_descriptors(ctx, type) < 0) {
    return -1;
  }

  type_new_set_slots(ctx, type);

  if (type_new_set_classcell(type) < 0) {
    return -1;
  }
  return 0;
}

static int slotdefs_initialized = 0;

#define TPSLOT(NAME, SLOT, FUNCTION, WRAPPER, DOC) \
  {NAME, offsetof(PyTypeObject, SLOT), (void *)(FUNCTION), WRAPPER, \
   PyDoc_STR(DOC)}

#define FLSLOT(NAME, SLOT, FUNCTION, WRAPPER, DOC, FLAGS) \
  {NAME, offsetof(PyTypeObject, SLOT), (void *)(FUNCTION), WRAPPER, \
    PyDoc_STR(DOC), FLAGS}

typedef struct wrapperbase slotdef;
PyObject *_PyType_LookupId(PyTypeObject *type, struct _Py_Identifier *name);

static PyObject *
lookup_maybe_method(PyObject *self, _Py_Identifier *attrid, int *unbound) {
  PyObject *res = _PyType_LookupId(Py_TYPE(self), attrid);
  if (res == NULL) {
    return NULL;
  }

  printf("lookup_maybe_method type is %s, name is %s\n", Py_TYPE(res)->tp_name, attrid->string);
  if (_PyType_HasFeature(Py_TYPE(res), Py_TPFLAGS_METHOD_DESCRIPTOR)) {
    *unbound = 1;
    Py_INCREF(res);
  } else {
    *unbound = 0;
    descrgetfunc f = Py_TYPE(res)->tp_descr_get;
    if (f == NULL) {
      Py_INCREF(res);
    } else {
      res = f(res, self, (PyObject *) Py_TYPE(self));
    }
  }
  return res;
}

static PyObject *
lookup_method(PyObject *self, _Py_Identifier *attrid, int *unbound) {
  PyObject *res = lookup_maybe_method(self, attrid, unbound);
  if (res == NULL && !PyErr_Occurred()) {
    assert(false);
  }
  return res;
}

PyObject *_PyObject_Call_Prepend(PyThreadState *tstate, PyObject *callable,
    PyObject *obj, PyObject *args, PyObject *kwargs);

static int
slot_tp_init(PyObject *self, PyObject *args, PyObject *kwds) {
  PyThreadState *tstate = _PyThreadState_GET();

  _Py_IDENTIFIER(__init__);
  int unbound;
  PyObject *meth = lookup_method(self, &PyId___init__, &unbound);
  if (meth == NULL) {
    return -1;
  }

  PyObject *res;
  if (unbound) {
    res = _PyObject_Call_Prepend(tstate, meth, self, args, kwds);
  } else {
    res = _PyObject_Call(tstate, meth, args, kwds);
  }
  Py_DECREF(meth);
  if (res == NULL)
    return -1;
  if (res != Py_None) {
    assert(false);
  }
  Py_DECREF(res);
  return 0;
}

static PyObject *
wrap_init(PyObject *self, PyObject *args, void *wrapped, PyObject *kwds) {
  assert(false);
}

static slotdef slotdefs[] = {
  TPSLOT("__getattribute__", tp_getattr, NULL, NULL, ""),
  FLSLOT("__init__", tp_init, slot_tp_init, (wrapperfunc) (void(*)(void)) wrap_init,
      "__init__", PyWrapperFlag_KEYWORDS),
  {NULL},
};

PyStatus
_PyTypes_InitSlotDefs(void) {
  if (slotdefs_initialized) {
    return _PyStatus_OK();
  }
  for (slotdef *p = slotdefs; p->name; p++) {
    // slots must be ordered by their offset in the PyHeapTypeObject
    assert(!p[1].name || p->offset <= p[1].offset);

    #if 0
    p->name_strobj = PyUnicode_InternFromString(p->name);
    if (!p->name_strobj || !PyUnicode_CHECK_INTERNED(p->name_strobj)) {
      assert(false);
    }
    #else
    p->name_strobj = PyUnicode_FromString(p->name);
    if (!p->name_strobj) {
      assert(false);
    }
    #endif
  }
  slotdefs_initialized = 1;
  return _PyStatus_OK();
}

static void **
slotptr(PyTypeObject *type, int ioffset) {
  char *ptr;
  long offset = ioffset;

  assert(offset >= 0);
  assert((size_t) offset < offsetof(PyHeapTypeObject, as_buffer));
  if ((size_t) offset >= offsetof(PyHeapTypeObject, as_sequence)) {
    assert(false); 
  } else if ((size_t) offset >= offsetof(PyHeapTypeObject, as_mapping)) {
    assert(false);
  } else if ((size_t) offset >= offsetof(PyHeapTypeObject, as_number)) {
    assert(false);
  } else if ((size_t) offset >= offsetof(PyHeapTypeObject, as_async)) {
    assert(false);
  } else {
    ptr = (char *) type;
  }
  if (ptr != NULL)
    ptr += offset;
  return (void **) ptr;
}

static slotdef *
update_one_slot(PyTypeObject *type, slotdef *p) {
  PyObject *descr;
  void *generic = NULL, *specific = NULL;
  int use_generic = 0;
  int offset = p->offset;
  int error;
  void **ptr = slotptr(type, offset);

  if (ptr == NULL) {
    assert(false);
  }
  assert(!PyErr_Occurred());
  do {
    descr = find_name_in_mro(type, p->name_strobj, &error);
    if (descr == NULL) {
      if (error == -1) {
        assert(false);
      }
      if (ptr == (void **)&type->tp_iternext) {
        specific = (void *) _PyObject_NextNotImplemented;
      }
      continue;
    }
    if (Py_IS_TYPE(descr, &PyWrapperDescr_Type) &&
        ((PyWrapperDescrObject *) descr)->d_base->name_strobj == p->name_strobj) {
      assert(false);
    } else if (Py_IS_TYPE(descr, &PyCFunction_Type) &&
        PyCFunction_GET_FUNCTION(descr) ==
        (PyCFunction)(void(*)(void))tp_new_wrapper &&
        ptr == (void **) &type->tp_new) {
      assert(false);
    } else if (descr == Py_None &&
        ptr == (void **)&type->tp_hash) {
      assert(false);
    } else {
      use_generic = 1;
      generic = p->function;
    }
  } while ((++p)->offset == offset);
  if (specific && !use_generic) {
    assert(false);
  } else {
    *ptr = generic;
  }
  return p;
}

static void
fixup_slot_dispatchers(PyTypeObject *type) {
  assert(!PyErr_Occurred());
  assert(slotdefs_initialized);
  for (slotdef *p = slotdefs; p->name; ) {
    p = update_one_slot(type, p);
  }
}

PyObject *
_PyType_LookupId(PyTypeObject *type, struct _Py_Identifier *name) {
  PyObject *oname;
  oname = _PyUnicode_FromId(name);
  if (oname == NULL)
    return NULL;
  return _PyType_Lookup(type, oname);
}

PyObject *
_PyObject_LookupSpecial(PyObject *self, _Py_Identifier *attrid) {
  PyObject *res;

  res = _PyType_LookupId(Py_TYPE(self), attrid);
  if (res != NULL) {
    assert(false);
  }
  return res;
}

// Call __set__name__ on all attributes
static int
type_new_set_names(PyTypeObject *type) {
  _Py_IDENTIFIER(__set_name__);
  PyObject *names_to_set = PyDict_Copy(type->tp_dict);
  if (names_to_set == NULL) {
    return -1;
  }

  Py_ssize_t i = 0;
  PyObject *key, *value;
  while (PyDict_Next(names_to_set, &i, &key, &value)) {
    PyObject *set_name = _PyObject_LookupSpecial(value, &PyId___set_name__);
    if (set_name == NULL) {
      if (PyErr_Occurred()) {
        assert(false);
      }
      continue;
    }
    assert(false);
  }

  Py_DECREF(names_to_set);
  return 0;
}

// Call __init_subclass__ on the parent of a newly generated type
static int
type_new_init_subclass(PyTypeObject *type, PyObject *kwds) {
  _Py_IDENTIFIER(__init_subclass__);
  PyObject *args[2] = {(PyObject *) type, (PyObject *) type};
  PyObject *super = _PyObject_FastCall((PyObject *) &PySuper_Type, args, 2);
  if (super == NULL) {
    return -1;
  }

  PyObject *func = _PyObject_GetAttrId(super, &PyId___init_subclass__);
  Py_DECREF(super);
  if (func == NULL) {
    return -1;
  }

  PyObject *result = PyObject_VectorcallDict(func, NULL, 0, kwds);
  Py_DECREF(func);
  if (result == NULL) {
    return -1;
  }

  Py_DECREF(result);
  return 0;
}

static PyObject *
type_new_impl(type_new_ctx *ctx) {
  PyTypeObject *type = type_new_init(ctx);
  if (type == NULL) {
    return NULL;
  }

  if (type_new_set_attrs(ctx, type) < 0) {
    assert(false);
  }

  if (PyType_Ready(type) < 0) {
    assert(false);
  }

  fixup_slot_dispatchers(type);

  if (type->tp_dictoffset) {
    PyHeapTypeObject *et = (PyHeapTypeObject *) type;
    et->ht_cached_keys = _PyDict_NewKeysForClass();
  }

  if (type_new_set_names(type) < 0) {
    assert(false);
  }

  if (type_new_init_subclass(type, ctx->kwds) < 0) {
    assert(false);
  }

  return (PyObject *) type;
}

static PyObject *type_new(PyTypeObject *metatype, PyObject *args, PyObject *kwds) {
  assert(args != NULL && PyTuple_Check(args));
  assert(kwds == NULL || PyDict_Check(kwds));

  PyObject *name, *bases, *orig_dict;
  if (!PyArg_ParseTuple(args, "UO!O!:type.__new__",
      &name,
      &PyTuple_Type, &bases,
      &PyDict_Type, &orig_dict))
  {
    return NULL;
  }

  type_new_ctx ctx = {
    .metatype = metatype,
    .args = args,
    .kwds = kwds,
    .orig_dict = orig_dict,
    .name = name,
    .bases = bases,
    .base = NULL,
    .slots = NULL,
    .nslot = 0,
    .add_dict = 0,
    .add_weak = 0,
    .may_add_dict = 0,
    .may_add_weak = 0,
  };
  PyObject *type = NULL;
  int res = type_new_get_bases(&ctx, &type);
  if (res < 0) {
    assert(false);
  }
  if (res == 1) {
    assert(false);
  }
  assert(ctx.base != NULL);
  assert(ctx.bases != NULL);

  type = type_new_impl(&ctx);
  Py_DECREF(ctx.bases);
  return type;
}

static int
excess_args(PyObject *args, PyObject *kwds) {
  return PyTuple_GET_SIZE(args) ||
      (kwds && PyDict_Check(kwds) && PyDict_GET_SIZE(kwds));
}

static int
object_init(PyObject *self, PyObject *args, PyObject *kwds) {
  PyTypeObject *type = Py_TYPE(self);
  if (excess_args(args, kwds)) {
    assert(false);
  }
  return 0;
}

static int type_init(PyObject *cls, PyObject *args, PyObject *kwds) {
  int res;

  assert(args != NULL && PyTuple_Check(args));
  assert(kwds == NULL || PyDict_Check(kwds));

  if (kwds != NULL && PyTuple_Check(args) && PyTuple_GET_SIZE(args) == 1 &&
      PyDict_Check(kwds) && PyDict_GET_SIZE(kwds) != 0) {
    assert(false);
  }

  if (args != NULL && PyTuple_Check(args) &&
      (PyTuple_GET_SIZE(args) != 1 && PyTuple_GET_SIZE(args) != 3)) {
    assert(false);
  }

  args = PyTuple_GetSlice(args, 0, 0);
  if (args == NULL) {
    return -1;
  }
  res = object_init(cls, args, NULL);
  Py_DECREF(args);
  return res;
}

PyObject *PyType_GenericAlloc(PyTypeObject *type, Py_ssize_t nitems) {
  PyObject *obj;
  const size_t size = _PyObject_VAR_SIZE(type, nitems + 1);
  /* note that we need to add one, for the sentinel */

  if (_PyType_IS_GC(type)) {
    obj = _PyObject_GC_Malloc(size);
  } else {
		obj = (PyObject *) PyObject_Malloc(size);
  }
  
  if (obj == NULL) {
    assert(false);
  }

  memset(obj, '\0', size);

  if (type->tp_itemsize == 0) {
    _PyObject_Init(obj, type);
  } else {
    assert(false);
  }

  if (_PyType_IS_GC(type)) {
    _PyObject_GC_TRACK(obj);
  }
  return obj;
}


PyObject *PyType_GenericNew(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  return type->tp_alloc(type, 0);
}

static PyTypeObject *
supercheck(PyTypeObject *type, PyObject *obj) {
  if (PyType_Check(obj) && PyType_IsSubtype((PyTypeObject*) obj, type)) {
    Py_INCREF(obj);
    return (PyTypeObject *) obj;
  }
  assert(false);
}

static int super_init(PyObject *self, PyObject *args, PyObject *kwds) {
  superobject *su = (superobject *) self;
  PyTypeObject *type = NULL;
  PyObject *obj = NULL;
  PyTypeObject *obj_type = NULL;

  if (!_PyArg_NoKeywords("super", kwds)) {
    return -1;
  }
  if (!PyArg_ParseTuple(args, "|O!O:super", &PyType_Type, &type, &obj))
    return -1;

  if (type == NULL) {
    assert(false);
  }

  if (obj == Py_None)
    obj = NULL;
  if (obj != NULL) {
    obj_type = supercheck(type, obj);
    if (obj_type == NULL)
      return -1;
    Py_INCREF(obj);
  }
  Py_INCREF(type);
  Py_XSETREF(su->type, type);
  Py_XSETREF(su->obj, obj);
  Py_XSETREF(su->obj_type, obj_type);
  return 0;
}

static PyObject *super_getattro(PyObject *self, PyObject *name) {
  _Py_IDENTIFIER(__class__);

  superobject *su = (superobject *) self;
  PyTypeObject *starttype;
  PyObject *mro;
  Py_ssize_t i, n;

  starttype = su->obj_type;
  if (starttype == NULL)
    goto skip;

  if (PyUnicode_Check(name) &&
      PyUnicode_GET_LENGTH(name) == 9 &&
      _PyUnicode_EqualToASCIIId(name, &PyId___class__))
    goto skip;

  mro = starttype->tp_mro;
  if (mro == NULL)
    goto skip;

  assert(PyTuple_Check(mro));
  n = PyTuple_GET_SIZE(mro);

  // no need to check the last one
  for (i = 0; i + 1 < n; i++) {
    if ((PyObject *) (su->type) == PyTuple_GET_ITEM(mro, i))
      break;
  }
  i++; // skip su->type (if any)
  if (i >= n)
    goto skip;

  Py_INCREF(mro);
  do {
    PyObject *res, *tmp, *dict;
    descrgetfunc f;

    tmp = PyTuple_GET_ITEM(mro, i);
    assert(PyType_Check(tmp));

    dict = ((PyTypeObject *) tmp)->tp_dict;
    assert(dict != NULL && PyDict_Check(dict));

    res = PyDict_GetItemWithError(dict, name);
    if (res != NULL) {
      Py_INCREF(res);

      f = Py_TYPE(res)->tp_descr_get;
      if (f != NULL) {
        tmp = f(res,
          (su->obj == (PyObject *) starttype) ? NULL : su->obj,
          (PyObject *) starttype);
        Py_DECREF(res);
        res = tmp;
      }

      Py_DECREF(mro);
      return res;
    } else if (PyErr_Occurred()) {
      Py_DECREF(mro);
      return NULL;
    }

    i++;
  } while (i < n);

  Py_DECREF(mro);

skip:
  return PyObject_GenericGetAttr(self, name);
}

static void super_dealloc(PyObject *self) {
  superobject *su = (superobject *) self;

  _PyObject_GC_UNTRACK(self);
  Py_XDECREF(su->obj);
  Py_XDECREF(su->type);
  Py_XDECREF(su->obj_type);
  Py_TYPE(self)->tp_free(self);
}

static int
add_operators(PyTypeObject *type) {
  PyObject *dict = type->tp_dict;
  slotdef *p;
  PyObject *descr;
  void **ptr;

  assert(slotdefs_initialized);
  for (p = slotdefs; p->name; p++) {
    if (p->wrapper == NULL)
      continue;
    ptr = slotptr(type, p->offset);
    if (!ptr || !*ptr)
      continue;
    int r = PyDict_Contains(dict, p->name_strobj);
    if (r > 0)
      continue;
    if (r < 0) {
      return -1;
    }
    if (*ptr == (void *) PyObject_HashNotImplemented) {
      assert(false);
    } else {
      descr = PyDescr_NewWrapper(type, p, *ptr);
      if (descr == NULL)
        return -1;
      if (PyDict_SetItem(dict, p->name_strobj, descr) < 0) {
        Py_DECREF(descr);
        return -1;
      }
      Py_DECREF(descr);
    }
  }
  return 0;
}
