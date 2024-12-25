#pragma once

static int
dict_merge(PyObject *a, PyObject *b, int override) {
	PyDictObject *mp, *other;
	Py_ssize_t i, n;
	PyDictKeyEntry *entry, *ep0;

	assert(0 <= override && override <= 2);

	if (a == NULL || !PyDict_Check(a) || b == NULL) {
		assert(false);
	}
	mp = (PyDictObject *) a;
	if (PyDict_Check(b) && (Py_TYPE(b)->tp_iter == (getiterfunc) dict_iter)) {
		other = (PyDictObject *) b;
		if (other == mp || other->ma_used == 0) {
			return 0;
		}
		if (mp->ma_used == 0) {
			override = 1;
			PyDictKeysObject *okeys = other->ma_keys;

			// TODO: follow cpy to do an optimization here
		}

		// TODO: do a big resize first
		ep0 = DK_ENTRIES(other->ma_keys);
		for (i = 0, n = other->ma_keys->dk_nentries; i < n; i++) {
			PyObject *key, *value;
			Py_hash_t hash;

			entry = &ep0[i];	
			key = entry->me_key;
			hash = entry->me_hash;
			if (other->ma_values) {
				value = other->ma_values[i];
			} else {
				value = entry->me_value;
			}
			if (value != NULL) {
				int err = 0;
				Py_INCREF(key);
				Py_INCREF(value);
				if (override == 1) {
					err = insertdict(mp, key, hash, value);
				} else {
					assert(false);
				}
				Py_DECREF(value);
				Py_DECREF(key);
				if (err != 0)
					return -1;
				if (n != other->ma_keys->dk_nentries) {
					assert(false && "dict mutated during update");
				}
			}
		}
	} else {
		// Do it the generic, slower way
		assert(false);
	}
	return 0;
}

int
PyDict_Update(PyObject *a, PyObject *b) {
	return dict_merge(a, b, 1);
}

static PyObject *dictiter_new(PyDictObject *dict, PyTypeObject *itertype);
typedef struct {
  PyObject_HEAD
  PyDictObject *di_dict;
  Py_ssize_t di_used;
  Py_ssize_t di_pos;
  PyObject *di_result;
  Py_ssize_t len;
} dictiterobject;

static void
dictiter_dealloc(dictiterobject *di) {
  Py_XDECREF(di->di_dict);
  Py_XDECREF(di->di_result);
  PyObject_GC_Del(di);
}

static PyObject *
dictiter_iternextkey(dictiterobject *di) {
  PyObject *key;
  Py_ssize_t i;
  PyDictKeysObject *k;
  PyDictObject *d = di->di_dict;

  if (d == NULL)
    return NULL;
  assert(PyDict_Check(d));

  if (di->di_used != d->ma_used) {
    assert(false);
  }

  i = di->di_pos;
  k = d->ma_keys;
  assert(i >= 0);
  if (d->ma_values) {
    assert(false);
  } else {
    Py_ssize_t n = k->dk_nentries;
    PyDictKeyEntry *entry_ptr = &DK_ENTRIES(k)[i];
    while (i < n && entry_ptr->me_value == NULL) {
      entry_ptr++;
      i++;
    }
    if (i >= n)
      goto fail;
    key = entry_ptr->me_key;
  }
  if (di->len == 0) {
    assert(false);
  }
  di->di_pos = i + 1;
  di->len--;
  Py_INCREF(key);
  return key;
fail:
  di->di_dict = NULL;
  Py_DECREF(d);
  return NULL;
}

PyTypeObject PyDictIterKey_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "dict_keyiterator",
  .tp_basicsize = sizeof(dictiterobject),
  .tp_dealloc = (destructor) dictiter_dealloc,
  .tp_getattro = PyObject_GenericGetAttr,
  .tp_iter = PyObject_SelfIter,
  .tp_iternext = (iternextfunc) dictiter_iternextkey,
};

static PyObject *dict_iter(PyDictObject *dict) {
  return dictiter_new(dict, &PyDictIterKey_Type);
}

Py_ssize_t
_PyDict_KeysSize(PyDictKeysObject *keys) {
  return (sizeof(PyDictKeysObject)
    + DK_IXSIZE(keys) * DK_SIZE(keys)
    + USABLE_FRACTION(DK_SIZE(keys)) * sizeof(PyDictKeyEntry));
}

static PyDictKeysObject *
clone_combined_dict_keys(PyDictObject *orig) {
  assert(PyDict_Check(orig));
  assert(Py_TYPE(orig)->tp_iter == (getiterfunc) dict_iter);
  assert(orig->ma_values == NULL);
  assert(orig->ma_keys->dk_refcnt == 1);

  Py_ssize_t keys_size = _PyDict_KeysSize(orig->ma_keys);
  PyDictKeysObject *keys = PyObject_Malloc(keys_size);
  if (keys == NULL) {
    assert(false);
  }

  memcpy(keys, orig->ma_keys, keys_size);

  PyDictKeyEntry *ep0 = DK_ENTRIES(keys);
  Py_ssize_t n = keys->dk_nentries;
  for (Py_ssize_t i = 0; i < n; i++) {
    PyDictKeyEntry *entry = &ep0[i];
    PyObject *value = entry->me_value;
    if (value != NULL) {
      Py_INCREF(value);
      Py_INCREF(entry->me_key);
    }
  }
  return keys;
}

PyObject *PyDict_Copy(PyObject *o) {
  PyObject *copy;
  PyDictObject *mp;

  if (o == NULL || !PyDict_Check(o)) {
    assert(false);
  }

  mp = (PyDictObject *) o;
  if (mp->ma_used == 0) {
    return PyDict_New();
  }

  if (_PyDict_HasSplitTable(mp)) {
    assert(false);
  }

  if (Py_TYPE(mp)->tp_iter == (getiterfunc) dict_iter &&
      mp->ma_values == NULL &&
      (mp->ma_used >= (mp->ma_keys->dk_nentries * 2) / 3)) {
    // fast copy
    PyDictKeysObject *keys = clone_combined_dict_keys(mp);
    if (keys == NULL) {
      return NULL;
    }
    PyDictObject *new = (PyDictObject *) new_dict(keys, NULL);
    if (new == NULL) {
      return NULL;
    }
    
    new->ma_used = mp->ma_used;
    return (PyObject *) new;
  }
  assert(false);
}

static PyObject *dictiter_new(PyDictObject *dict, PyTypeObject *itertype) {
  dictiterobject *di;
  di = PyObject_GC_New(dictiterobject, itertype);
  if (di == NULL) {
    return NULL;
  }
  Py_INCREF(dict);
  di->di_dict = dict;
  di->di_used = dict->ma_used;
  di->len = dict->ma_used;
  if (itertype == &PyDictIterKey_Type) {
    di->di_pos = 0;
  } else {
    assert(false);
  }

  if (itertype == &PyDictIterKey_Type) {
    di->di_result = NULL;
  } else {
    assert(false);
  }
  return (PyObject *) di;
}


