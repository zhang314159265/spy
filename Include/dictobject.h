static PyObject *empty_values[1] = { NULL };

typedef struct _dictkeysobject PyDictKeysObject;

#define PyDict_MINSIZE 8
#define PERTURB_SHIFT 5

// defined in cpy/Include/cpython/dictobject.h
typedef struct {
  PyObject_HEAD

  Py_ssize_t ma_used;

  uint64_t ma_version_tag;
  PyDictKeysObject *ma_keys;
  PyObject **ma_values;
} PyDictObject;

#define _PyDict_HasSplitTable(d) ((d)->ma_values != NULL)

#define GROWTH_RATE(d) ((d)->ma_used * 3)

#include "Objects/dict-common.h"
#include "Objects/stringlib/eq.h"

Py_ssize_t lookdict_split(PyDictObject *mp, PyObject *key,
    Py_hash_t hash, PyObject **value_addr);
static int insertion_resize(PyDictObject *mp);

// defined in cpy/Objects/dictobject.c
static PyDictKeysObject empty_keys_struct = {
  .dk_refcnt = 1,
  .dk_lookup = lookdict_split,
  .dk_size = 1, // TODO why 1 rather than 8?
  .dk_indices = {
    DKIX_EMPTY, DKIX_EMPTY, DKIX_EMPTY, DKIX_EMPTY, 
    DKIX_EMPTY, DKIX_EMPTY, DKIX_EMPTY, DKIX_EMPTY, 
  },
};

#define Py_EMPTY_KEYS &empty_keys_struct

#define PyDict_Check(op) \
  PyType_FastSubclass(Py_TYPE(op), Py_TPFLAGS_DICT_SUBCLASS)

#define PyDict_CheckExact(op) Py_IS_TYPE(op, &PyDict_Type)

#define DK_SIZE(dk) ((dk)->dk_size)

#if SIZEOF_VOID_P > 4
#define DK_IXSIZE(dk) \
  (DK_SIZE(dk) <= 0xff ? \
    1 : DK_SIZE(dk) <= 0xffff ? \
      2 : DK_SIZE(dk) <= 0xffffffff ? \
      4 : sizeof(int64_t))
#else
#error SIZEOF_VOID_P <= 4
#endif

#define DK_ENTRIES(dk) \
  ((PyDictKeyEntry*) (&((int8_t*) ((dk)->dk_indices))[DK_SIZE(dk) * DK_IXSIZE(dk)]))

// defined in cpy/Objects/dictobject.c
#define DK_MASK(dk) (((dk)->dk_size) - 1)

static void dict_dealloc(PyDictObject *mp);
static PyObject *dict_repr(PyDictObject *mp);
static int dict_ass_sub(PyDictObject *mp, PyObject *v, PyObject *w);
static PyObject *dict_subscript(PyDictObject *mp, PyObject *key);

static PyMappingMethods dict_as_mapping = {
  .mp_subscript = (binaryfunc) dict_subscript,
  .mp_ass_subscript = (objobjargproc) dict_ass_sub,
};

PyTypeObject PyDict_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "dict",
  .tp_basicsize = sizeof(PyDictObject),
  .tp_flags = Py_TPFLAGS_DICT_SUBCLASS,
  .tp_dealloc = (destructor) dict_dealloc,
  .tp_free = PyObject_GC_Del,
  .tp_repr = (reprfunc) dict_repr,
  .tp_as_mapping = &dict_as_mapping,
};

static inline Py_ssize_t
calculate_keysize(Py_ssize_t minsize) {
  Py_ssize_t size;
  for (size = PyDict_MINSIZE;
      size < minsize && size > 0;
      size <<= 1)
    ;
  return size;
}

static inline void
dictkeys_incref(PyDictKeysObject *dk) {
  dk->dk_refcnt++;
}

static void
free_keys_object(PyDictKeysObject *keys) {
  PyDictKeyEntry *entries = DK_ENTRIES(keys);
  Py_ssize_t i, n;
  for (i = 0, n = keys->dk_nentries; i < n; i++) {
    Py_XDECREF(entries[i].me_key);
    Py_XDECREF(entries[i].me_value);
  }

  PyObject_Free(keys);
}

static inline void
dictkeys_decref(PyDictKeysObject *dk) {
  assert(dk->dk_refcnt > 0);
  if (--dk->dk_refcnt == 0) {
    free_keys_object(dk);
  }
}

static uint64_t pydict_global_version = 0;

#define DICT_NEXT_VERSION() (++pydict_global_version)

static PyObject *
new_dict(PyDictKeysObject *keys, PyObject **values)
{
  PyDictObject *mp;
  assert(keys != NULL);

  mp = PyObject_GC_New(PyDictObject, &PyDict_Type);
  assert(mp);
  mp->ma_keys = keys;
  mp->ma_values = values;
  mp->ma_used = 0;
  mp->ma_version_tag = DICT_NEXT_VERSION();
  // ASSERT_CONSISTENT(mp);
  return (PyObject *) mp;
}

// defined in cpy/Objects/dictobject.c
PyObject *PyDict_New(void) {
  dictkeys_incref(Py_EMPTY_KEYS);
  return new_dict(Py_EMPTY_KEYS, empty_values);
}

PyObject *PyDict_GetItemWithError(PyObject *op, PyObject *key) {
  Py_ssize_t ix;
  Py_hash_t hash;
  PyDictObject *mp = (PyDictObject *) op;
  PyObject *value;

  if (!PyDict_Check(op)) {
    assert(false);
  }
  if (!PyUnicode_CheckExact(key) ||
      (hash = ((PyASCIIObject *) key)->hash) == -1) {
    hash = PyObject_Hash(key);
    if (hash == -1) {
      return NULL;
    }
  }

  ix = (mp->ma_keys->dk_lookup)(mp, key, hash, &value);
  if (ix < 0) {
    return NULL;
  }
  return value;
}

static inline Py_ssize_t
dictkeys_get_index(const PyDictKeysObject *keys, Py_ssize_t i) {
  Py_ssize_t s = DK_SIZE(keys);
  Py_ssize_t ix;

  if (s <= 0xff) {
    const int8_t *indices = (const int8_t*) (keys->dk_indices);
    ix = indices[i];
  } else if (s <= 0xffff) {
    assert(false);
  }
#if SIZEOF_VOID_P > 4
  else if (s > 0xffffffff) {
    assert(false);
  }
#endif
  else {
    assert(false);
  }
  assert(ix >= DKIX_DUMMY);
  return ix;
}

static Py_ssize_t
lookdict(PyDictObject *mp, PyObject *key,
    Py_hash_t hash, PyObject **value_addr) {
  size_t i, mask, perturb;
  PyDictKeysObject *dk;
  PyDictKeyEntry *ep0;

top:
  dk = mp->ma_keys;
  ep0 = DK_ENTRIES(dk);
  mask = DK_MASK(dk);
  perturb = hash;
  i = (size_t) hash & mask;

  for (;;) {
    Py_ssize_t ix = dictkeys_get_index(dk, i);
    if (ix == DKIX_EMPTY) {
      *value_addr = NULL;
      return ix;
    }
    if (ix >= 0) {
      PyDictKeyEntry *ep = &ep0[ix];
      assert(ep->me_key != NULL);
      if (ep->me_key == key) {
        *value_addr = ep->me_value;
        return ix;
      }
      if (ep->me_hash == hash) {
        PyObject *startkey = ep->me_key;
        Py_INCREF(startkey);
        int cmp = PyObject_RichCompareBool(startkey, key, Py_EQ);
        Py_DECREF(startkey);
        if (cmp < 0) {
          *value_addr = NULL;
          return DKIX_ERROR;
        }
        if (dk == mp->ma_keys && ep->me_key == startkey) {
          if (cmp > 0) {
            *value_addr = ep->me_value;
            return ix;
          }
        } else {
          assert(false);
        }
      }
    }
    perturb >>= PERTURB_SHIFT;
    i = (i * 5 + perturb + 1) & mask;
  }
  Py_UNREACHABLE();
}

Py_ssize_t lookdict_split(PyDictObject *mp, PyObject *key,
    Py_hash_t hash, PyObject **value_addr) {
  /* mp must split table */
  assert(mp->ma_values != NULL);
  if (!PyUnicode_CheckExact(key)) {
    Py_ssize_t ix = lookdict(mp, key, hash, value_addr);
    if (ix >= 0) {
      *value_addr = mp->ma_values[ix];
    }
    return ix;
  }
  PyDictKeyEntry *ep0 = DK_ENTRIES(mp->ma_keys);
  size_t mask = DK_MASK(mp->ma_keys);
  size_t perturb = (size_t) hash;
  size_t i = (size_t) hash & mask;

  for (;;) {
    Py_ssize_t ix = dictkeys_get_index(mp->ma_keys, i);
    assert(ix != DKIX_DUMMY);
    if (ix == DKIX_EMPTY) {
      *value_addr = NULL;
      return DKIX_EMPTY;
    }
    assert(false);
  }
  assert(false);
}


#define USABLE_FRACTION(n) (((n) << 1) / 3)

#define IS_POWER_OF_2(x) (((x) & (x - 1)) == 0)

#define PyDict_GET_SIZE(mp) (assert(PyDict_Check(mp)), ((PyDictObject*) mp)->ma_used)

static Py_ssize_t
lookdict_unicode_nodummy(PyDictObject *mp, PyObject *key,
    Py_hash_t hash, PyObject **value_addr) {
  assert(mp->ma_values == NULL);
  if (!PyUnicode_CheckExact(key)) {
    return lookdict(mp, key, hash, value_addr);
  }

  PyDictKeyEntry *ep0 = DK_ENTRIES(mp->ma_keys);
  size_t mask = DK_MASK(mp->ma_keys);
  size_t perturb = (size_t) hash;
  size_t i = (size_t) hash & mask;

  for (;;) {
    Py_ssize_t ix = dictkeys_get_index(mp->ma_keys, i);
    assert(ix != DKIX_DUMMY);
    if (ix == DKIX_EMPTY) {
      *value_addr = NULL;
      return DKIX_EMPTY;
    }
    PyDictKeyEntry *ep = &ep0[ix];
    assert(ep->me_key != NULL);
    assert(PyUnicode_CheckExact(ep->me_key));
    if (ep->me_key == key ||
        (ep->me_hash == hash && unicode_eq(ep->me_key, key))) {
      *value_addr = ep->me_value;
      return ix;
    }
    perturb >>= PERTURB_SHIFT;
    i = mask & (i * 5 + perturb + 1);
  }
  assert(false); // can not reach here
}

static PyDictKeysObject*
new_keys_object(Py_ssize_t size) {
  PyDictKeysObject *dk;
  Py_ssize_t es, usable;

  assert(size >= PyDict_MINSIZE);
  assert(IS_POWER_OF_2(size));

  usable = USABLE_FRACTION(size);
  if (size <= 0xff) {
    es = 1;
  } else if (size <= 0xffff) {
    es = 2;
  }
#if SIZEOF_VOID_P > 4
  else if (size <= 0xffffffff) {
    es = 4;
  }
#endif
  else {
    es = sizeof(Py_ssize_t);
  }

  dk = PyObject_Malloc(sizeof(PyDictKeysObject)
    + es * size + sizeof(PyDictKeyEntry) * usable);
  if (dk == NULL) {
    assert(false);
  }
  dk->dk_refcnt = 1;
  dk->dk_size = size;
  dk->dk_usable = usable;
  dk->dk_lookup = lookdict_unicode_nodummy;
  dk->dk_nentries = 0;
  memset(&dk->dk_indices[0], 0xff, es * size);
  memset(DK_ENTRIES(dk), 0, sizeof(PyDictKeyEntry) * usable);
  return dk;
}

static inline void
dictkeys_set_index(PyDictKeysObject *keys, Py_ssize_t i, Py_ssize_t ix) {
  Py_ssize_t s = DK_SIZE(keys);

  assert(ix >= DKIX_DUMMY);

  if (s <= 0xff) {
    int8_t *indices = (int8_t*) (keys->dk_indices);
    assert(ix <= 0x7f);
    indices[i] = (char) ix;
  } else {
    assert(false);
  }
}

static Py_ssize_t find_empty_slot(PyDictKeysObject *keys, Py_hash_t hash) ;

static int
insertdict(PyDictObject *mp, PyObject *key, Py_hash_t hash, PyObject *value) {
  PyObject *old_value;
  PyDictKeyEntry *ep;

  Py_INCREF(key);
  Py_INCREF(value);
  if (mp->ma_values != NULL && !PyUnicode_CheckExact(key)) {
    assert(false);
  }

  Py_ssize_t ix = mp->ma_keys->dk_lookup(mp, key, hash, &old_value);
  if (ix == DKIX_ERROR) {
    assert(false);
  }

  /* When insertion order is different from shared key, we can't share
   * the key anymore.  Convert this instance to combine table.
   */
  if (_PyDict_HasSplitTable(mp)) {
    assert(false);
  }

  if (ix == DKIX_EMPTY) {
    /* Insert into new slot. */
    assert(old_value == NULL);
    if (mp->ma_keys->dk_usable <= 0) {
      // Need to resize
      if (insertion_resize(mp) < 0)
        assert(false);
    }
    if (!PyUnicode_CheckExact(key) && mp->ma_keys->dk_lookup != lookdict) {
      mp->ma_keys->dk_lookup = lookdict;
    }
    Py_ssize_t hashpos = find_empty_slot(mp->ma_keys, hash);
    ep = &DK_ENTRIES(mp->ma_keys)[mp->ma_keys->dk_nentries];
    dictkeys_set_index(mp->ma_keys, hashpos, mp->ma_keys->dk_nentries);
    ep->me_key = key;
    ep->me_hash = hash;
    if (mp->ma_values) {
      assert(false);
    } else {
      ep->me_value = value;
    }
    mp->ma_used++;
    mp->ma_version_tag = DICT_NEXT_VERSION();
    mp->ma_keys->dk_usable--;
    mp->ma_keys->dk_nentries++;
    assert(mp->ma_keys->dk_usable >= 0);
    return 0;
  }
  
  if (old_value != value) {
    if (_PyDict_HasSplitTable(mp)) {
      assert(false);
    }
    else {
      assert(old_value != NULL);
      DK_ENTRIES(mp->ma_keys)[ix].me_value = value;
    }
    mp->ma_version_tag = DICT_NEXT_VERSION();
  }

  Py_XDECREF(old_value);
  Py_DECREF(key);
  return 0;
}

// Same to insertdict but specialized for ma_keys = Py_EMPTY_KEYS
static int
insert_to_emptydict(PyDictObject *mp, PyObject *key, Py_hash_t hash,
    PyObject *value) {
  assert(mp->ma_keys == Py_EMPTY_KEYS);

  PyDictKeysObject *newkeys = new_keys_object(PyDict_MINSIZE);
  if (newkeys == NULL) {
    return -1;
  }
  if (!PyUnicode_CheckExact(key)) {
    newkeys->dk_lookup = lookdict;
  }
  dictkeys_decref(Py_EMPTY_KEYS);
  mp->ma_keys = newkeys;
  mp->ma_values = NULL;

  Py_INCREF(key);
  Py_INCREF(value);

  size_t hashpos = (size_t) hash & (PyDict_MINSIZE - 1);
  PyDictKeyEntry *ep = DK_ENTRIES(mp->ma_keys);
  dictkeys_set_index(mp->ma_keys, hashpos, 0);
  ep->me_key = key;
  ep->me_hash = hash;
  ep->me_value = value;
  mp->ma_used++;
  mp->ma_version_tag = DICT_NEXT_VERSION();
  mp->ma_keys->dk_usable--;
  mp->ma_keys->dk_nentries++;
  return 0;
}

int PyDict_SetItem(PyObject *op, PyObject *key, PyObject *value) {
  PyDictObject *mp;
  Py_hash_t hash;
  if (!PyDict_Check(op)) {
    assert(false);
  }
  assert(key);
  assert(value);
  mp = (PyDictObject *) op;
  if (!PyUnicode_CheckExact(key) ||
      (hash = ((PyASCIIObject *)  key)->hash) == -1) {
    hash = PyObject_Hash(key);
    if (hash == -1)
      return -1;
  }
  if (mp->ma_keys == Py_EMPTY_KEYS) {
    return insert_to_emptydict(mp, key, hash, value);
  }
  // insertdict handles any resizing that might be necessary
  return insertdict(mp, key, hash, value);
}

int
_PyDict_Next(PyObject *op, Py_ssize_t *ppos, PyObject **pkey,
    PyObject **pvalue, Py_hash_t *phash) {
  Py_ssize_t i;
  PyDictObject *mp;
  PyDictKeyEntry *entry_ptr;
  PyObject *value;

  if (!PyDict_Check(op))
    return 0;
  mp = (PyDictObject *) op;
  i = *ppos;
  if (mp->ma_values) {
    if (i < 0 || i >= mp->ma_used)
      return 0;
    assert(false);
  } else {
    Py_ssize_t n = mp->ma_keys->dk_nentries;
    if (i < 0 || i >= n)
      return 0;
    entry_ptr = &DK_ENTRIES(mp->ma_keys)[i];
    while (i < n && entry_ptr->me_value == NULL) {
      entry_ptr++;
      i++;
    }
    if (i >= n)
      return 0;
    value = entry_ptr->me_value;
  }
  *ppos = i + 1;
  if (pkey)
    *pkey = entry_ptr->me_key;
  if (phash)
    *phash = entry_ptr->me_hash;
  if (pvalue)
    *pvalue = value;
  return 1;
}

int PyDict_Next(
    PyObject *op, Py_ssize_t *ppos, PyObject **pkey, PyObject **pvalue) {
  return _PyDict_Next(op, ppos, pkey, pvalue, NULL);
}

static void
dict_dealloc(PyDictObject *mp) {
  PyObject **values = mp->ma_values;
  PyDictKeysObject *keys = mp->ma_keys;

  if (values != NULL) {
    if (values != empty_values) {
      assert(false);
    }
    dictkeys_decref(keys);
  } else if (keys != NULL) {
    assert(keys->dk_refcnt == 1);
    dictkeys_decref(keys);
  }

  Py_TYPE(mp)->tp_free((PyObject *) mp);
}

static Py_ssize_t 
find_empty_slot(PyDictKeysObject *keys, Py_hash_t hash) {
  assert(keys != NULL);

  const size_t mask = DK_MASK(keys);
  size_t i = hash & mask;
  Py_ssize_t ix = dictkeys_get_index(keys, i);
  for (size_t perturb = hash; ix >=0 ;) {
    perturb >>= PERTURB_SHIFT;
    i = (i * 5 + perturb + 1) & mask;
    ix = dictkeys_get_index(keys, i);
  }
  return i;
}

// Internal routine used by dictresize() to build a hashtable of entries.
static void
build_indices(PyDictKeysObject *keys, PyDictKeyEntry *ep, Py_ssize_t n) {
  size_t mask = (size_t) DK_SIZE(keys) - 1;
  for (Py_ssize_t ix = 0; ix != n; ix++, ep++) {
    Py_hash_t hash = ep->me_hash;
    size_t i = hash & mask;
    for (size_t perturb = hash; dictkeys_get_index(keys, i) != DKIX_EMPTY;) {
      perturb >>= PERTURB_SHIFT;
      i = mask & (i * 5 + perturb + 1);
    }
    dictkeys_set_index(keys, i, ix);
  }
}

static int
dictresize(PyDictObject *mp, Py_ssize_t newsize)
{
  Py_ssize_t numentries;
  PyDictKeysObject *oldkeys;
  PyObject **oldvalues;
  PyDictKeyEntry *oldentries, *newentries;

  if (newsize <= 0) {
    assert(false);
  }
  assert(IS_POWER_OF_2(newsize));
  assert(newsize >= PyDict_MINSIZE);

  oldkeys = mp->ma_keys;

  mp->ma_keys = new_keys_object(newsize);
  if (mp->ma_keys == NULL) {
    assert(false);
  }
  // New table must be large enough
  assert(mp->ma_keys->dk_usable >= mp->ma_used);
  if (oldkeys->dk_lookup == lookdict)
    mp->ma_keys->dk_lookup = lookdict;

  numentries = mp->ma_used;
  oldentries = DK_ENTRIES(oldkeys);
  newentries = DK_ENTRIES(mp->ma_keys);
  oldvalues = mp->ma_values;
  if (oldvalues != NULL) {
    assert(false);
  } else {  // combined table.
    if (oldkeys->dk_nentries == numentries) {
      memcpy(newentries, oldentries, numentries * sizeof(PyDictKeyEntry));
    } else {
      PyDictKeyEntry *ep = oldentries;
      for (Py_ssize_t i = 0; i < numentries; i++) {
        while (ep->me_value == NULL)
          ep++;
        newentries[i] = *ep++;
      }
    }

    assert(oldkeys->dk_lookup != lookdict_split);
    assert(oldkeys->dk_refcnt == 1);
    PyObject_Free(oldkeys);
  }

  build_indices(mp->ma_keys, newentries, numentries);
  mp->ma_keys->dk_usable -= numentries;
  mp->ma_keys->dk_nentries = numentries;
  return 0;
}

static int
insertion_resize(PyDictObject *mp)
{
  return dictresize(mp, calculate_keysize(GROWTH_RATE(mp)));
}

PyObject *
PyDict_SetDefault(PyObject *d, PyObject *key, PyObject *defaultobj) {
  PyDictObject *mp = (PyDictObject *) d;
  PyObject *value;
  Py_hash_t hash;
  
  if (!PyDict_Check(d)) {
    assert(false);
  }
  if (!PyUnicode_CheckExact(key) ||
      (hash = ((PyASCIIObject *) key)->hash) == -1) {
    hash = PyObject_Hash(key);
    if (hash == -1)
      return NULL;
  }

  if (mp->ma_keys == Py_EMPTY_KEYS) {
    if (insert_to_emptydict(mp, key, hash, defaultobj) < 0) {
      return NULL;
    }
    return defaultobj;
  }
  if (mp->ma_values != NULL && !PyUnicode_CheckExact(key)) {
    assert(false);
  }

  Py_ssize_t ix = (mp->ma_keys->dk_lookup)(mp, key, hash, &value);
  if (ix == DKIX_ERROR)
    return NULL;

  if (_PyDict_HasSplitTable(mp)) {
    assert(false);
  }

  if (ix == DKIX_EMPTY) {
    PyDictKeyEntry *ep, *ep0;
    value = defaultobj;
    if (mp->ma_keys->dk_usable <= 0) {
      // printf("mp->ma_used %ld\n", mp->ma_used);
      if (insertion_resize(mp) < 0) {
        return NULL;
      }
    }

    if (!PyUnicode_CheckExact(key) && mp->ma_keys->dk_lookup != lookdict) {
      mp->ma_keys->dk_lookup = lookdict;
    }
    Py_ssize_t hashpos = find_empty_slot(mp->ma_keys, hash);
    ep0 = DK_ENTRIES(mp->ma_keys);
    ep = &ep0[mp->ma_keys->dk_nentries];
    dictkeys_set_index(mp->ma_keys, hashpos, mp->ma_keys->dk_nentries);
    Py_INCREF(key);
    Py_INCREF(value);
    ep->me_key = key;
    ep->me_hash = hash;
    if (_PyDict_HasSplitTable(mp)) {
      assert(false);
    } else {
      ep->me_value = value;
    }
    mp->ma_used++;
    mp->ma_version_tag = DICT_NEXT_VERSION();
    mp->ma_keys->dk_usable--;
    mp->ma_keys->dk_nentries++;
    assert(mp->ma_keys->dk_usable >= 0);
  } else if (value == NULL) {
    assert(false);
  }

  return value;
}

PyObject *
_PyDict_GetItem_KnownHash(PyObject *op, PyObject *key, Py_hash_t hash) {
  Py_ssize_t ix;
  PyDictObject *mp = (PyDictObject *) op;
  PyObject *value;

  if (!PyDict_Check(op)) {
    assert(false);
  }

  ix = (mp->ma_keys->dk_lookup)(mp, key, hash, &value);
  if (ix < 0) {
    return NULL;
  }
  return value;
}

PyObject *
_PyDict_GetItemIdWithError(PyObject *dp, struct _Py_Identifier *key)
{
  PyObject *kv;
  kv = _PyUnicode_FromId(key);
  if (kv == NULL)
    return NULL;
  Py_hash_t hash = ((PyASCIIObject *) kv)->hash;
  assert(hash != -1);
  return _PyDict_GetItem_KnownHash(dp, kv, hash);
}

int
_PyDict_SetItemId(PyObject *v, struct _Py_Identifier *key, PyObject *item) {
  PyObject *kv;
  kv = _PyUnicode_FromId(key);
  if (kv == NULL)
    return -1;
  return PyDict_SetItem(v, kv, item);
}

int
_PyObjectDict_SetItem(PyTypeObject *tp, PyObject **dictptr,
    PyObject *key, PyObject *value) {
  PyObject *dict;
  int res;

  assert(dictptr != NULL);

  if ((tp->tp_flags & Py_TPFLAGS_HEAPTYPE)) {
    assert(false);
  } else {
    dict = *dictptr;
    if (dict == NULL) {
      assert(false);
    }
    if (value == NULL) {
      assert(false);
    } else {
      res = PyDict_SetItem(dict, key, value);
    }
  }
  return res;
}

int
PyDict_SetItemString(PyObject *v, const char *key, PyObject *item) {
  PyObject *kv;
  int err;
  kv = PyUnicode_FromString(key);
  if (kv == NULL)
    return -1;
  PyUnicode_InternInPlace(&kv);

  err = PyDict_SetItem(v, kv, item);
  Py_DECREF(kv);
  return err;
}

PyObject *
_PyDict_LoadGlobal(PyDictObject *globals, PyDictObject *builtins, PyObject *key) {
  Py_ssize_t ix;
  Py_hash_t hash;
  PyObject *value;

  if (!PyUnicode_CheckExact(key) ||
      (hash = ((PyASCIIObject *) key)->hash) == -1) {
    hash = PyObject_Hash(key);
    if (hash == -1)
      return NULL;
  }

  // namespace 1: globals
  ix = globals->ma_keys->dk_lookup(globals, key, hash, &value);
  if (ix == DKIX_ERROR)
    return NULL;
  if(ix != DKIX_EMPTY && value != NULL)
    return value;

  // namespace 2: builtins
  ix = builtins->ma_keys->dk_lookup(builtins, key, hash, &value);
  if (ix < 0)
    return NULL;
  return value;
}

PyObject *
_PyDict_NewPresized(Py_ssize_t minused) {

  if (minused <= USABLE_FRACTION(PyDict_MINSIZE)) {
    return PyDict_New();
  }
  assert(false);
}

static PyObject *dict_repr(PyDictObject *mp) {
  Py_ssize_t i;
  PyObject *key = NULL, *value = NULL;
  _PyUnicodeWriter writer;
  int first;

  if (mp->ma_used == 0) {
    return PyUnicode_FromString("{}");
  }

  _PyUnicodeWriter_Init(&writer);
  writer.overallocate = 1;
  writer.min_length = 1 + 4 + (2 + 4) * (mp->ma_used - 1) + 1;

  if (_PyUnicodeWriter_WriteChar(&writer, '{') < 0)
    assert(false);
  i = 0;
  first = 1;
  while (PyDict_Next((PyObject *) mp, &i, &key, &value)) {
    PyObject *s;
    int res;

    Py_INCREF(key);
    Py_INCREF(value);

    if (!first) {
      assert(false);
    }
    first = 0;

    s = PyObject_Repr(key);
    if (s == NULL)
      assert(false);
    res = _PyUnicodeWriter_WriteStr(&writer, s);
    Py_DECREF(s);
    if (res < 0)
      assert(false);

    if (_PyUnicodeWriter_WriteASCIIString(&writer, ": ", 2) < 0)
      assert(false);

    s = PyObject_Repr(value);
    if (s == NULL)
      assert(false);

    res = _PyUnicodeWriter_WriteStr(&writer, s);
    Py_DECREF(s);
    if (res < 0)
      assert(false);
    Py_CLEAR(key);
    Py_CLEAR(value);
  }

  writer.overallocate = 0;
  if (_PyUnicodeWriter_WriteChar(&writer, '}') < 0)
    assert(false);

  return _PyUnicodeWriter_Finish(&writer);
}

static int dict_ass_sub(PyDictObject *mp, PyObject *v, PyObject *w) {
  if (w == NULL) {
    assert(false);
  } else {
    return PyDict_SetItem((PyObject *) mp, v, w);
  }
}

static PyObject *dict_subscript(PyDictObject *mp, PyObject *key) {
  Py_ssize_t ix;
  Py_hash_t hash;
  PyObject *value;

  if (!PyUnicode_CheckExact(key) ||
      (hash = ((PyASCIIObject *) key)->hash) == -1) {
    hash = PyObject_Hash(key);
    if (hash == -1)
      return NULL;
  }
  ix = (mp->ma_keys->dk_lookup)(mp, key, hash, &value);
  if (ix == DKIX_ERROR)
    return NULL;
  if (ix == DKIX_EMPTY || value == NULL) {
    assert(false);
  }
  Py_INCREF(value);
  return value;
}

