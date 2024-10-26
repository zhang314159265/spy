#pragma once
// no such header in cpy

#include "tupleobject.h"
#include "methodobject.h"


static int type_ready(PyTypeObject *type);
static int type_ready_mro(PyTypeObject *type);

static void
object_dealloc(PyObject *self)
{
	if (Py_TYPE(self)->tp_free == NULL) {
		printf("dealloc a object of type %s with NULL tp_Free\n", Py_TYPE(self)->tp_name);
	}
	Py_TYPE(self)->tp_free(self);
}

PyObject *PyType_GenericAlloc(PyTypeObject *type, Py_ssize_t nitems);

PyDoc_STRVAR(object_init_subclass_doc, "");

// defined in cpy/Objects/typeobject.c
static PyObject *
object_init_subclass(PyObject *cls, PyObject *arg) {
  Py_RETURN_NONE;
}

static PyMethodDef object_methods[] = {
  {"__init_subclass__", object_init_subclass, METH_CLASS | METH_NOARGS,
    object_init_subclass_doc},
  {0},
};

static PyObject *object_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int object_init(PyObject *self, PyObject *args, PyObject *kwds);

// defined in cpy/Objects/typeobject.c
PyTypeObject PyBaseObject_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "object",
  .tp_basicsize = sizeof(PyObject),
	.tp_dealloc = object_dealloc,
	.tp_free = PyObject_Del,
	.tp_hash = (hashfunc) _Py_HashPointer,
  .tp_alloc = PyType_GenericAlloc,
  .tp_methods = object_methods,
  .tp_new = object_new,
  .tp_init = object_init,
  .tp_getattro = PyObject_GenericGetAttr,
  .tp_setattro = PyObject_GenericSetAttr,
};

static int
pmerge(PyObject *acc, PyObject **to_merge, Py_ssize_t to_merge_size) {
	int res = 0;
	Py_ssize_t i, j, empty_cnt;
  int *remain;

	remain = PyMem_New(int, to_merge_size);
	if (remain == NULL) {
		assert(false);
	}
	for (i = 0; i < to_merge_size; i++) {
		remain[i] = 0;
	}
 again:
 	empty_cnt = 0;
	for (i = 0; i < to_merge_size; i++) {
		PyObject *candidate;

		PyObject *cur_tuple = to_merge[i];

		if (remain[i] >= PyTuple_GET_SIZE(cur_tuple)) {
			empty_cnt++;
			continue;
		}
		assert(false);
	}

	if (empty_cnt != to_merge_size) {
		assert(false);
	}
 out:
 	PyMem_Free(remain);

	return res;
}

static int
type_ready_set_dict(PyTypeObject *type) {
  if (type->tp_dict != NULL) {
    return 0;
  }

  PyObject *dict = PyDict_New();
  if (dict == NULL) {
    return -1;
  }
  type->tp_dict = dict;
  return 0;
}

// defined in cpy/Objects/typeobject.c
int PyType_Ready(PyTypeObject *type) {
  if (type->tp_flags & Py_TPFLAGS_READY) {
    return 0;
  }
  type->tp_flags |= Py_TPFLAGS_READYING;

  if (type_ready(type) < 0) {
    type->tp_flags &= ~Py_TPFLAGS_READYING;
    return -1;
  }

  // All done -- set the ready flag
  type->tp_flags = (type->tp_flags & ~Py_TPFLAGS_READYING) | Py_TPFLAGS_READY;
  return 0;
}



static int
type_ready_set_bases(PyTypeObject *type) {
  PyTypeObject *base = type->tp_base;
  if (base == NULL && type != &PyBaseObject_Type) {
    base = &PyBaseObject_Type;
    if (type->tp_flags & Py_TPFLAGS_HEAPTYPE) {
      assert(false);
    } else {
      type->tp_base = base;
    }
  }
  assert(type->tp_base != NULL || type == &PyBaseObject_Type);

  // Initialize the base class
  if (base != NULL && !_PyType_IsReady(base)) {
    if (PyType_Ready(base) < 0) {
      return -1;
    }
  }

  if (Py_IS_TYPE(type, NULL) && base != NULL) {
    Py_SET_TYPE(type, Py_TYPE(base));
  }

  // Initialize tp_bases
  PyObject *bases = type->tp_bases;
  if (bases == NULL) {
    PyTypeObject *base = type->tp_base;
    if (base == NULL) {
      bases = PyTuple_New(0);
    } else {
      bases = PyTuple_Pack(1, base);
    }
    if (bases == NULL) {
      return -1;
    }
    type->tp_bases = bases;
  }
  return 0;
}

static void
inherit_special(PyTypeObject *type, PyTypeObject *base) {
	if (type->tp_basicsize == 0)
		type->tp_basicsize = base->tp_basicsize;
}

static int
overrides_hash(PyTypeObject *type) {
	// not implemented yet
	return 0;
}

static int
inherit_slots(PyTypeObject *type, PyTypeObject *base) {

#undef SLOTDEFINED
#undef COPYSLOT

#define SLOTDEFINED(SLOT) \
	(base->SLOT != 0 && \
		(basebase == NULL || base->SLOT != basebase->SLOT))

#define COPYSLOT(SLOT) \
	if (!type->SLOT && SLOTDEFINED(SLOT)) type->SLOT = base->SLOT

	PyTypeObject *basebase;
	printf("inherit slots from %s for %s\n", base->tp_name, type->tp_name);

	basebase = base->tp_base;

	COPYSLOT(tp_dealloc);
  if (type->tp_getattr == NULL && type->tp_getattro == NULL) {
    type->tp_getattr = base->tp_getattr;
    type->tp_getattro = base->tp_getattro;
  }
  if (type->tp_setattr == NULL && type->tp_setattro == NULL) {
    type->tp_setattr = base->tp_setattr;
    type->tp_setattro = base->tp_setattro;
  }
	{
		if (type->tp_richcompare == NULL && type->tp_hash == NULL) {
			int r = overrides_hash(type);
			if (r < 0) {
				return -1;
			} 
			if (!r) {
				type->tp_richcompare = base->tp_richcompare;
				type->tp_hash = base->tp_hash;
			}
		}
	}
  {
    COPYSLOT(tp_alloc);
  }
	return 0;
}

static int
type_ready_inherit(PyTypeObject *type) {
	// printf("type_ready_inherit for %s\n", type->tp_name);
	PyTypeObject *base = type->tp_base;
	if (base != NULL) {
		inherit_special(type, base);
	}

	// Inherit slots
	PyObject *mro = type->tp_mro;
	Py_ssize_t n = PyTuple_GET_SIZE(type->tp_mro);
	for (Py_ssize_t i = 1; i < n; i++) {
		PyObject *b = PyTuple_GET_ITEM(mro, i);
		if (PyType_Check(b)) {
			if (inherit_slots(type, (PyTypeObject *) b) < 0) {
				return -1;
			}
		}
	}

	if (base != NULL) {
		// TODO inherit as_structs
	}
	return 0;
}

static int type_add_methods(PyTypeObject *type);
static int add_operators(PyTypeObject *type);

static int
type_ready_fill_dict(PyTypeObject *type) {
  if (add_operators(type) < 0) {
    return -1;
  }
	if (type_add_methods(type) < 0) {
		return -1;
	}
	return 0;
}

static PyObject *
tp_new_wrapper(PyObject *self, PyObject *args, PyObject *kwds) {
  assert(false);
}

static struct PyMethodDef tp_new_methoddef[] = {
  {"__new__", (PyCFunction)(void(*)(void)) tp_new_wrapper, METH_VARARGS | METH_KEYWORDS,
    PyDoc_STR("")},
  {0},
};

static int
add_tp_new_wrapper(PyTypeObject *type) {
  _Py_IDENTIFIER(__new__);
  int r = _PyDict_ContainsId(type->tp_dict, &PyId___new__);
  if (r > 0) {
    return 0;
  }
  if (r < 0) {
    return -1;
  }

  PyObject *func = PyCFunction_NewEx(tp_new_methoddef, (PyObject *) type, NULL);
  if (func == NULL) {
    return -1;
  }
  r = _PyDict_SetItemId(type->tp_dict, &PyId___new__, func);
  Py_DECREF(func);
  return r;
}

static int
type_ready_set_new(PyTypeObject *type) {
  PyTypeObject *base = type->tp_base;
  if (type->tp_new == NULL
      && base == &PyBaseObject_Type
      && !(type->tp_flags & Py_TPFLAGS_HEAPTYPE)) {
    type->tp_flags |= Py_TPFLAGS_DISALLOW_INSTANTIATION;
  }

  if (!(type->tp_flags & Py_TPFLAGS_DISALLOW_INSTANTIATION)) {
    if (type->tp_new != NULL) {
      if (add_tp_new_wrapper(type) < 0) {
        return -1;
      }
    } else {
      type->tp_new = base->tp_new;
    }
  } else {
    type->tp_new = NULL;
  }
  return 0;
}

static int
type_ready(PyTypeObject *type) {
  printf("type_ready: %s\n", type->tp_name);
  if (type_ready_set_dict(type) < 0) {
    return -1;
  }
  if (type_ready_set_bases(type) < 0) {
    return -1;
  }
  if (type_ready_mro(type) < 0) {
    return -1;
  }
  if (type_ready_set_new(type) < 0) {
    return -1;
  }
	if (type_ready_fill_dict(type) < 0) {
		return -1;
	}
	if (type_ready_inherit(type) < 0) {
		return -1;
	}
	return 0;
}


// defined in cpy/Objects/typeobject.c
static PyObject * mro_implementation(PyTypeObject *type) {
	PyObject *result;
  PyObject *bases;
	PyObject **to_merge;
  Py_ssize_t i, n;

  if (!_PyType_IsReady(type)) {
    if (PyType_Ready(type) < 0)
      return NULL;
  }

  bases = type->tp_bases;
  assert(PyTuple_Check(bases));
  n = PyTuple_GET_SIZE(bases);
	for (int i = 0; i < n; i++) {
		PyTypeObject *base = (PyTypeObject *) PyTuple_GET_ITEM(bases, i);
		if (base->tp_mro == NULL) {
			assert(false);
		}
		assert(PyTuple_Check(base->tp_mro));
	}
	if (n == 1) {
		// Fast path
		PyTypeObject *base = (PyTypeObject *) PyTuple_GET_ITEM(bases, 0);
		Py_ssize_t k = PyTuple_GET_SIZE(base->tp_mro);
		result = PyTuple_New(k + 1);
		if (result == NULL) {
			return NULL;
		}
		Py_INCREF(type);
		PyTuple_SET_ITEM(result, 0, (PyObject *) type);
		for (i = 0; i < k; i++) {
			PyObject *cls = PyTuple_GET_ITEM(base->tp_mro, i);
			Py_INCREF(cls);
			PyTuple_SET_ITEM(result, i + 1, cls);
		}
		return result;
	}
	to_merge = PyMem_New(PyObject *, n + 1);
	if (to_merge == NULL) {
		assert(false);
	}

	for (i = 0; i < n; i++) {
		PyTypeObject *base = (PyTypeObject *) PyTuple_GET_ITEM(bases, i);
		to_merge[i] = base->tp_mro;
	}
	to_merge[n] = bases;

	result = PyList_New(1);
	if (result == NULL) {
		assert(false);
	}

	Py_INCREF(type);
	PyList_SET_ITEM(result, 0, (PyObject *) type);
	if (pmerge(result, to_merge, n + 1) < 0) {
		assert(false);
	}

	PyMem_Free(to_merge);
	return result;
}



static PyObject *
mro_invoke(PyTypeObject *type) {
  PyObject *mro_result;
  PyObject *new_mro;
  const int custom = !Py_IS_TYPE(type, &PyType_Type);
  if (custom) {
    assert(false);
  } else {
    mro_result = mro_implementation(type);
  }
  if (mro_result == NULL)
    return NULL;

  new_mro = PySequence_Tuple(mro_result);
  Py_DECREF(mro_result);
  if (new_mro == NULL) {
    return NULL;
  }
  // printf("type is %s\n", type->tp_name);

  if (PyTuple_GET_SIZE(new_mro) == 0) {
    assert(false);
  }

  if (custom) {
    assert(false);
  }
  return new_mro;
}

static int
mro_internal(PyTypeObject *type, PyObject **p_old_mro) {
  PyObject *new_mro, *old_mro;
  int reent;

  old_mro = type->tp_mro;
  Py_XINCREF(old_mro);
  new_mro = mro_invoke(type); // might cause reentrance
  reent = (type->tp_mro != old_mro);
	Py_XDECREF(old_mro);
	if (new_mro == NULL) {
		return -1;
	}

	if (reent) {
		assert(false);
	}

	type->tp_mro = new_mro;

	// call type_mro_modofied
	// call PyType_Modified(type)

	if (p_old_mro != NULL)
		*p_old_mro = old_mro; // transfer the ownership
	else
		Py_XDECREF(old_mro);
	
	return 1;
}

static int
type_ready_mro(PyTypeObject *type) {
  // Calculate method resolution order
  if (mro_internal(type, NULL) < 0) {
    return -1;
  }
	assert(type->tp_mro != NULL);
	assert(PyTuple_Check(type->tp_mro));
	return 0;
}

extern PyTypeObject PyBaseObject_Type;

int PyType_Ready(PyTypeObject *type);

PyObject *PyTuple_Pack(Py_ssize_t, ...);
PyObject *PyTuple_New(Py_ssize_t size);

static PyObject *
find_name_in_mro(PyTypeObject *type, PyObject *name, int *error) {
	Py_ssize_t i, n;
	PyObject *mro, *res, *base, *dict;
	Py_hash_t hash;

	if (!PyUnicode_CheckExact(name) ||
			(hash = ((PyASCIIObject *) name)->hash) == -1) {
	  hash = PyObject_Hash(name);
		if (hash == -1) {
			*error = -1;
			return NULL;
		}
	}

	mro = type->tp_mro;

	if (mro == NULL) {
    if ((type->tp_flags & Py_TPFLAGS_READYING) == 0) {
      if (PyType_Ready(type) < 0) {
        *error = -1;
        return NULL;
      }
      mro = type->tp_mro;
    }
    if (mro == NULL) {
      *error = 1;
      return NULL;
    }
	}

	res = NULL;
	Py_INCREF(mro);
	assert(PyTuple_Check(mro));
	n = PyTuple_GET_SIZE(mro);
	for (i = 0; i < n; i++) {
		base = PyTuple_GET_ITEM(mro, i);
		assert(PyType_Check(base));
		dict = ((PyTypeObject *) base)->tp_dict;
		assert(dict && PyDict_Check(dict));
		res = _PyDict_GetItem_KnownHash(dict, name, hash);
		if (res != NULL)
			break;
		if (PyErr_Occurred()) {
			*error = -1;
			goto done;
		}
	}
	*error = 0;
done:
	Py_DECREF(mro);
	return res;
}

PyObject *
_PyType_Lookup(PyTypeObject *type, PyObject *name) {
	PyObject *res;
	int error;

	res = find_name_in_mro(type, name, &error);
	if (error) {
		assert(false);
	}
	return res;
}

static int excess_args(PyObject *args, PyObject *kwds);


static PyObject *object_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  if (excess_args(args, kwds)) {
    if (type->tp_new != object_new) {
      assert(false);
    }
    if (type->tp_init == object_init) {
      assert(false);
    }
  }

  if (type->tp_flags & Py_TPFLAGS_IS_ABSTRACT) {
    assert(false);
  }
  return type->tp_alloc(type, 0);
}
