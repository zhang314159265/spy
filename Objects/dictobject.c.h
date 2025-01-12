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

int _PyDict_MergeEx(PyObject *a, PyObject *b, int override) {
  return dict_merge(a, b, override);
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
dictiter_iternextitem(dictiterobject *di) {
  PyObject *key, *value, *result;
  Py_ssize_t i;
  PyDictObject *d = di->di_dict;

  if (d == NULL)
    return NULL;
  assert(PyDict_Check(d));

  if (di->di_used != d->ma_used) {
    assert(false);
  }

  i = di->di_pos;
  assert(i >= 0);
  if (d->ma_values) {
    assert(false);
  } else {
    Py_ssize_t n = d->ma_keys->dk_nentries;
    PyDictKeyEntry *entry_ptr = &DK_ENTRIES(d->ma_keys)[i];
    while (i < n && entry_ptr->me_value == NULL) {
      entry_ptr++;
      i++;
    }
    if (i >= n)
      goto fail;
    key = entry_ptr->me_key;
    value = entry_ptr->me_value;
  }
  if (di->len == 0) {
    assert(false);
  }
  di->di_pos = i + 1;
  di->len--;
  Py_INCREF(key);
  Py_INCREF(value);
  result = di->di_result;
  if (Py_REFCNT(result) == 1) {
    PyObject *oldkey = PyTuple_GET_ITEM(result, 0);
    PyObject *oldvalue = PyTuple_GET_ITEM(result, 1);
    PyTuple_SET_ITEM(result, 0, key);
    PyTuple_SET_ITEM(result, 1, value);
    Py_INCREF(result);
    Py_DECREF(oldkey);
    Py_DECREF(oldvalue);
  } else {
    result = PyTuple_New(2);
    if (result == NULL)
      return NULL;
    PyTuple_SET_ITEM(result, 0, key);
    PyTuple_SET_ITEM(result, 1, value);
  }
  return result;
fail:
  di->di_dict = NULL;
  Py_DECREF(d);
  return NULL;

  fail(0);
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

static PyObject *dictiter_new(PyDictObject *dict, PyTypeObject *itertype);

PyObject *_PyDict_GetItemStringWithError(PyObject *v, const char *key) {
  PyObject *kv, *rv;
  kv = PyUnicode_FromString(key);
  if (kv == NULL) {
    return NULL;
  }
  rv = PyDict_GetItemWithError(v, kv);
  Py_DECREF(kv);
  return rv;
}

PyObject *PyObject_GenericGetDict(PyObject *obj, void *context) {
  PyObject *dict, **dictptr = _PyObject_GetDictPtr(obj);
  if (dictptr == NULL) {
    assert(false);
  }
  dict = *dictptr;
  if (dict == NULL) {
    PyTypeObject *tp = Py_TYPE(obj);
    *dictptr = dict = PyDict_New();
  }
  Py_XINCREF(dict);
  return dict;
}

int
PyDict_Merge(PyObject *a, PyObject *b, int override) {
  return dict_merge(a, b, override != 0);
}

static int
dict_update_arg(PyObject *self, PyObject *arg) {
  if (PyDict_CheckExact(arg)) {
    return PyDict_Merge(self, arg, 1);
  }
  assert(false);
}

static int
dict_update_common(PyObject *self, PyObject *args, PyObject *kwds,
    const char *methname) {
  PyObject *arg = NULL;
  int result = 0;

  if (!PyArg_UnpackTuple(args, methname, 0, 1, &arg)) {
    result = -1;
  } else if (arg != NULL) {
    result = dict_update_arg(self, arg);
  }
  if (result == 0 && kwds != NULL) {
    assert(false);
  }
  return result;
}

static PyObject *
dict_update(PyObject *self, PyObject *args, PyObject *kwds) {
  if (dict_update_common(self, args, kwds, "update") != -1)
    Py_RETURN_NONE;
  return NULL;
}

PyObject *
_PyDictView_New(PyObject *dict, PyTypeObject *type) {
  _PyDictViewObject *dv;
  if (dict == NULL) {
    fail(0);
  }
  if (!PyDict_Check(dict)) {
    fail(0);
  }
  dv = PyObject_GC_New(_PyDictViewObject, type);
  if (dv == NULL)
    return NULL;
  Py_INCREF(dict);
  dv->dv_dict = (PyDictObject *) dict;
  return (PyObject *) dv;
}

static void
dictview_dealloc(_PyDictViewObject *dv) {
  Py_XDECREF(dv->dv_dict);
  PyObject_GC_Del(dv);
}

static PyObject *
dictiter_iternextitem(dictiterobject *di);

PyTypeObject PyDictIterItem_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "dict_itemiterator",
  .tp_basicsize = sizeof(dictiterobject),
  .tp_dealloc = (destructor) dictiter_dealloc,
  .tp_getattro = PyObject_GenericGetAttr,
  .tp_iter = PyObject_SelfIter,
  .tp_iternext = (iternextfunc) dictiter_iternextitem,
};

static PyObject *
dictitems_iter(_PyDictViewObject *dv) {
  if (dv->dv_dict == NULL) {
    Py_RETURN_NONE;
  }
  return dictiter_new(dv->dv_dict, &PyDictIterItem_Type);
}

PyTypeObject PyDictItems_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "dict_items",
  .tp_basicsize = sizeof(_PyDictViewObject),
  .tp_dealloc = (destructor) dictview_dealloc,
  // .tp_as_sequence = &dictitems_as_sequence,
  .tp_getattro = PyObject_GenericGetAttr,
  .tp_iter = (getiterfunc) dictitems_iter,
  // .tp_getset = dictview_getset,
};

static PyObject *
dictitems_new(PyObject *dict, PyObject *ignored) {
  return _PyDictView_New(dict, &PyDictItems_Type);
}

static PyObject *dict_pop(PyDictObject *self, PyObject *const *args, Py_ssize_t nargs);

static PyObject *dict_get(PyDictObject *self, PyObject *const *args, Py_ssize_t nargs);

#define DICT_POP_METHODDEF \
  {"pop", (PyCFunction)(void(*)(void))dict_pop, METH_FASTCALL, ""},

#define DICT_GET_METHODDEF \
  {"get", (PyCFunction)(void(*)(void))dict_get, METH_FASTCALL, ""},

static PyMethodDef mapp_methods[] = {
  {"update", (PyCFunction)(void(*)(void)) dict_update, METH_VARARGS | METH_KEYWORDS, ""},
  {"items", dictitems_new, METH_NOARGS, ""},
  DICT_POP_METHODDEF
  DICT_GET_METHODDEF
  {NULL, NULL}
};


static PySequenceMethods dict_as_sequence = {
  .sq_contains = PyDict_Contains,
};

PyTypeObject PyDict_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "dict",
  .tp_basicsize = sizeof(PyDictObject),
  .tp_flags = Py_TPFLAGS_DICT_SUBCLASS,
  .tp_dealloc = (destructor) dict_dealloc,
  .tp_free = PyObject_GC_Del,
  .tp_repr = (reprfunc) dict_repr,
  .tp_as_sequence = &dict_as_sequence,
  .tp_as_mapping = &dict_as_mapping,
	.tp_iter = (getiterfunc) dict_iter,
  .tp_methods = mapp_methods,
};



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
  if (itertype == &PyDictIterKey_Type || itertype == &PyDictIterItem_Type) {
    di->di_pos = 0;
  } else {
    assert(false);
  }

  if (itertype == &PyDictIterKey_Type) {
    di->di_result = NULL;
  } else if (itertype == &PyDictIterItem_Type) {
    di->di_result = PyTuple_Pack(2, Py_None, Py_None);
    if (di->di_result == NULL) {
      fail(0);
    }
  } else {
    assert(false);
  }
  return (PyObject *) di;
}

PyObject *
_PyDict_Pop_KnownHash(PyObject *dict, PyObject *key, Py_hash_t hash, PyObject *deflt) {
  Py_ssize_t ix, hashpos;
  PyObject *old_value, *old_key;
  PyDictKeyEntry *ep;
  PyDictObject *mp;

  assert(PyDict_Check(dict));
  mp = (PyDictObject *) dict;

  if (mp->ma_used == 0) {
    if (deflt) {
      Py_INCREF(deflt);
      return deflt;
    }
    fail(0);
  }
  ix = (mp->ma_keys->dk_lookup)(mp, key, hash, &old_value);
  if (ix == DKIX_ERROR)
    return NULL;
  if (ix == DKIX_EMPTY || old_value == NULL) {
    if (deflt) {
      Py_INCREF(deflt);
      return deflt;
    }
    fail(0);
  }
  if (_PyDict_HasSplitTable(mp)) {
    fail(0);
  }

  hashpos = lookdict_index(mp->ma_keys, hash, ix);
  assert(hashpos >= 0);
  assert(old_value != NULL);
  mp->ma_used--;
  mp->ma_version_tag = DICT_NEXT_VERSION();
  dictkeys_set_index(mp->ma_keys, hashpos, DKIX_DUMMY);
  ep = &DK_ENTRIES(mp->ma_keys)[ix];
  ENSURE_ALLOWS_DELETIONS(mp);
  old_key = ep->me_key;
  ep->me_key = NULL;
  ep->me_value = NULL;
  Py_DECREF(old_key);

  return old_value;
}

PyObject *
_PyDict_Pop(PyObject *dict, PyObject *key, PyObject *deflt) {
  Py_hash_t hash;

  if (((PyDictObject *)dict)->ma_used == 0) {
    fail(0);
  }
  if (!PyUnicode_CheckExact(key) ||
      (hash = ((PyASCIIObject *) key)->hash) == -1) {
    hash = PyObject_Hash(key);
    if (hash == -1)
      return NULL;
  }
  return _PyDict_Pop_KnownHash(dict, key, hash, deflt);
}

static PyObject *
dict_pop_impl(PyDictObject *self, PyObject *key, PyObject *default_value) {
  return _PyDict_Pop((PyObject *) self, key, default_value);
}

static PyObject *dict_pop(PyDictObject *self, PyObject *const *args, Py_ssize_t nargs) {
  PyObject *return_value = NULL;
  PyObject *key;
  PyObject *default_value = NULL;

  if (!_PyArg_CheckPositional("pop", nargs, 1, 2)) {
    goto exit;
  }
  key = args[0];
  if (nargs < 2) {
    goto skip_optional;
  }
  default_value = args[1];
skip_optional:
  return_value = dict_pop_impl(self, key, default_value);

exit:
  return return_value;
}

static PyObject *
dict_get_impl(PyDictObject *self, PyObject *key, PyObject *default_value) {
  PyObject *val = NULL;
  Py_hash_t hash;
  Py_ssize_t ix;

  if (!PyUnicode_CheckExact(key) ||
      (hash = ((PyASCIIObject *) key)->hash) == -1) {
    fail(0);
  }
  ix = (self->ma_keys->dk_lookup)(self, key, hash, &val);
  if (ix == DKIX_ERROR)
    return NULL;
  if (ix == DKIX_EMPTY || val == NULL) {
    val = default_value;
  }
  Py_INCREF(val);
  return val;
}


static PyObject *dict_get(PyDictObject *self, PyObject *const *args, Py_ssize_t nargs) {
  PyObject *return_value = NULL;
  PyObject *key;
  PyObject *default_value = Py_None;

  if (!_PyArg_CheckPositional("get", nargs, 1, 2)) {
    goto exit;
  }
  key = args[0];
  if (nargs < 2) {
    goto skip_optional;
  }
  default_value = args[1];
skip_optional:
  return_value = dict_get_impl(self, key, default_value);
exit:
  return return_value;
}
