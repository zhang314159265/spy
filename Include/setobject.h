#pragma once

#define PySet_MINSIZE 8

#define PyAnySet_Check(ob) \
  (Py_IS_TYPE(ob, &PySet_Type) || Py_IS_TYPE(ob, &PyFrozenSet_Type) || \
    PyType_IsSubtype(Py_TYPE(ob), &PySet_Type) || \
    PyType_IsSubtype(Py_TYPE(ob), &PyFrozenSet_Type))

#define PySet_Check(ob) \
	(Py_IS_TYPE(ob, &PySet_Type) || \
	PyType_IsSubtype(Py_TYPE(ob), &PySet_Type))

#define PySet_GET_SIZE(so) (assert(PyAnySet_Check(so)), (((PySetObject *) (so))->used))

// Object used as dummy key to fill deleted entries
static PyObject _dummy_struct;
#define dummy (&_dummy_struct)

typedef struct {
	PyObject *key;
	Py_hash_t hash; // cached hash code of the key
} setentry;

// defined in cpy/Include/setobject.h
typedef struct {
	PyObject_HEAD

	setentry *table;
	setentry smalltable[PySet_MINSIZE];

	Py_ssize_t fill; // Number active and dummy entries
  Py_ssize_t used; // number active entries

  Py_ssize_t mask;
  PyObject *weakreflist;
} PySetObject;


static PyObject *set_ior(PySetObject *so, PyObject *other);

static PyNumberMethods set_as_number = {
  .nb_inplace_or = (binaryfunc) set_ior,
};

typedef struct {
  PyObject_HEAD
  PySetObject *si_set; // set to NULL when iterator is exhausted
  Py_ssize_t si_used;
  Py_ssize_t si_pos;
  Py_ssize_t len;
} setiterobject;

static PyObject *setiter_iternext(setiterobject *si);
static void setiter_dealloc(setiterobject *si);

PyTypeObject PySetIter_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "set_iterator",
  .tp_basicsize = sizeof(setiterobject),
  .tp_itemsize = 0,
  .tp_iternext = (iternextfunc) setiter_iternext,
  .tp_dealloc = (destructor) setiter_dealloc,
};

static PyObject *
set_iter(PySetObject *so) {
  setiterobject *si = PyObject_GC_New(setiterobject, &PySetIter_Type);
  if (si == NULL)
    return NULL;
  Py_INCREF(so);
  si->si_set = so;
  si->si_used = so->used;
  si->si_pos = 0;
  si->len = so->used;
  _PyObject_GC_TRACK(si);
  return (PyObject *) si;
}

static void
set_dealloc(PySetObject *so) {
  setentry *entry;
  Py_ssize_t used = so->used;

  if (so->weakreflist != NULL) {
    assert(false);
  }

  for (entry = so->table; used > 0; entry++) {
    if (entry->key && entry->key != dummy) {
      used--;
      Py_DECREF(entry->key);
    }
  }
  if (so->table != so->smalltable)
    PyMem_Free(so->table);
  Py_TYPE(so)->tp_free(so);
}

PyTypeObject PySet_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "set",
	.tp_basicsize = sizeof(PySetObject),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_HAVE_GC,
	.tp_alloc = PyType_GenericAlloc,
  .tp_as_number = &set_as_number,
  .tp_iter = (getiterfunc) set_iter,
  .tp_dealloc = (destructor) set_dealloc,
  .tp_free = PyObject_GC_Del,
};

PyTypeObject PyFrozenSet_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "frozenset",
  .tp_basicsize = sizeof(PySetObject),
  .tp_itemsize = 0,
  .tp_flags = 0,
};

static int
set_merge(PySetObject *so, PyObject *otherset) {
  PySetObject *other;
	PyObject *key;
	Py_ssize_t i;
	setentry *so_entry;
	setentry *other_entry;

  assert(PyAnySet_Check(so));
  assert(PyAnySet_Check(otherset));

  other = (PySetObject*) otherset;
  if (other == so || other->used == 0)
    // a.update(a) or a.update(set()); nothing to do
    return 0;
	
	// printf("so used %ld, other used %ld\n", so->used, other->used);
	if ((so->fill + other->used) * 5 >= so->mask * 3) {
		assert(false);
	}

	so_entry = so->table;
	other_entry = other->table;

	if (so->fill == 0 && so->mask == other->mask && other->fill == other->used) {
		for (i = 0; i <= other->mask; i++, so_entry++, other_entry++) {
			key = other_entry->key;
			if (key != NULL) {
				assert(so_entry->key == NULL);
				Py_INCREF(key);
				so_entry->key = key;
				so_entry->hash = other_entry->hash;
			}
		}
		so->fill = other->fill;
		so->used = other->used;
		return 0;
	}
  assert(false);
}

static int
set_update_internal(PySetObject *so, PyObject *other)
{
  if (PyAnySet_Check(other))
    return set_merge(so, other);
  assert(false);
}

static PyObject *
set_ior(PySetObject *so, PyObject *other) {
  if (!PyAnySet_Check(other)) {
    assert(false);
  }

  if (set_update_internal(so, other))
    return NULL;
  Py_INCREF(so);
  return (PyObject *) so;
}

static PyObject *
make_new_set(PyTypeObject *type, PyObject *iterable) {
	assert(PyType_Check(type));
	PySetObject *so;

	so = (PySetObject *) type->tp_alloc(type, 0);
	if (so == NULL) {
		return NULL;
	}

	so->fill = 0;
  so->used = 0;
  so->mask = PySet_MINSIZE - 1;
	so->table = so->smalltable;
  so->weakreflist = NULL;

	if (iterable != NULL) {
		if (set_update_internal(so, iterable)) {
			Py_DECREF(so);
			return NULL;
		}
	}

	return (PyObject *) so;
}

// defined in cpy/Objects/setobject.c
PyObject *PySet_New(PyObject *iterable) {
	return make_new_set(&PySet_Type, iterable);
}

#define LINEAR_PROBES 9

#define PERTURB_SHIFT 5

static setentry *
set_lookkey(PySetObject *so, PyObject *key, Py_hash_t hash) {
  setentry *entry;
  size_t perturb = hash;
  size_t mask = so->mask;
  size_t i = (size_t) hash & mask;
  int probes;

  while (1) {
    entry = &so->table[i];
    probes = (i + LINEAR_PROBES <= mask) ? LINEAR_PROBES : 0;
    do {
      if (entry->hash == 0 && entry->key == NULL)
        return entry;
      if (entry->hash == hash) {
        assert(false);
      }
      entry++;
    } while (probes--);
    perturb >>= PERTURB_SHIFT;
    i = (i * 5 + 1 + perturb) & mask;
  }
}

static int
set_contains_entry(PySetObject *so, PyObject *key, Py_hash_t hash) {
  setentry *entry;

  entry = set_lookkey(so, key, hash);
  if (entry != NULL) {
    return entry->key != NULL;
  }
  return -1;
}

static int
set_contains_key(PySetObject *so, PyObject *key) {
  Py_hash_t hash;

  if (!PyUnicode_CheckExact(key) ||
      (hash = ((PyASCIIObject *) key)->hash) == -1) {
    assert(false);
  }
  return set_contains_entry(so, key, hash);
}

// defined in cpy/Objects/setobject.c
int PySet_Contains(PyObject *anyset, PyObject *key) {
  if (!PyAnySet_Check(anyset)) {
    assert(false);
  }
  return set_contains_key((PySetObject *) anyset, key);
}

static PyObject *setiter_iternext(setiterobject *si) {
  PyObject *key;
  Py_ssize_t i, mask;
  setentry *entry;
  PySetObject *so = si->si_set;

  if (so == NULL)
    return NULL;
  assert(PyAnySet_Check(so));

  if (si->si_used != so->used) {
    assert(false);
  }

  i = si->si_pos;
  assert(i >= 0);
  entry = so->table;
  mask = so->mask;
  while (i <= mask && (entry[i].key == NULL || entry[i].key == dummy))
    i++;
  si->si_pos = i + 1;
  if (i > mask)
    goto fail;
  si->len--;
  key = entry[i].key;
  Py_INCREF(key);
  return key;

fail:
  si->si_set = NULL;
  Py_DECREF(so);
  return NULL;
}

static void setiter_dealloc(setiterobject *si) {
  _PyObject_GC_UNTRACK(si);
  Py_XDECREF(si->si_set);
  PyObject_GC_Del(si);
}

static int
set_add_entry(PySetObject *so, PyObject *key, Py_hash_t hash) {
	setentry *freeslot;
	setentry *entry;

	size_t perturb;
	size_t mask;
	size_t i;
	int probes;

	Py_INCREF(key);

restart:

	mask = so->mask;
	i = (size_t) hash & mask;
	freeslot = NULL;
	perturb = hash;

	while (1) {
		entry = &so->table[i];
		probes = (i + LINEAR_PROBES <= mask) ? LINEAR_PROBES : 0;
		do {
			if (entry->hash == 0 && entry->key == NULL)
				goto found_unused_or_dummy;
			if (entry->hash == hash) {
				assert(false);
			}
			else if (entry->hash == -1) {
				assert(false);
			}
			entry++;
		} while (probes--);
		perturb >>= PERTURB_SHIFT;
		i = (i * 5 + 1 + perturb) & mask;
	}
found_unused_or_dummy:
	if (freeslot == NULL)
		goto found_unused;
	assert(false);
found_unused:
	so->fill++;
	so->used++;
	entry->key = key;
	entry->hash = hash;
	if ((size_t) so->fill * 5 < mask * 3)
		return 0;
	assert(false);
}

static int
set_add_key(PySetObject *so, PyObject *key) {
	Py_hash_t hash;

	if (!PyUnicode_CheckExact(key) ||
		(hash = ((PyASCIIObject *) key)->hash) == -1) {
		hash = PyObject_Hash(key);
		if (hash == -1)
			return -1;
	}
	return set_add_entry(so, key, hash);
}

int PySet_Add(PyObject *anyset, PyObject *key) {
	if (!PySet_Check(anyset)) {
		assert(false);
	}
	return set_add_key((PySetObject *) anyset, key);
}

#define DISCARD_NOTFOUND 0
#define DISCARD_FOUND 1

static int
set_discard_entry(PySetObject *so, PyObject *key, Py_hash_t hash) {
	setentry *entry;

	entry = set_lookkey(so, key, hash);
	if (entry == NULL)
		return -1;
	if (entry->key == NULL)
		return DISCARD_NOTFOUND;
	assert(false);
}

static int
set_discard_key(PySetObject *so, PyObject *key) {
	Py_hash_t hash;

	if (!PyUnicode_CheckExact(key) ||
			(hash = ((PyASCIIObject *) key)->hash) == -1) {
		hash = PyObject_Hash(key);
		if (hash == -1)
			return -1;
	}

	return set_discard_entry(so, key, hash);
}

int PySet_Discard(PyObject *set, PyObject *key) {
	if (!PySet_Check(set)) {
		assert(false);
	}
	return set_discard_key((PySetObject *) set, key);
}
