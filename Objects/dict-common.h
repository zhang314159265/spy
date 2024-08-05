#define DKIX_EMPTY (-1)
#define DKIX_DUMMY (-2)
#define DKIX_ERROR (-3)

typedef struct {
  /* Cached hash code of me_key. */
  Py_hash_t me_hash;
  PyObject *me_key;
  PyObject *me_value; /* This field is only meaningful for combined tables */
} PyDictKeyEntry;

typedef Py_ssize_t (*dict_lookup_func)
  (PyDictObject *mp, PyObject *key, Py_hash_t hash, PyObject **value_addr);

struct _dictkeysobject {
  Py_ssize_t dk_refcnt;

  /* Size (capacity) of the hash table (dk_indices). It must be a power of 2. */
  Py_ssize_t dk_size;

  // Number of usabled entries in dk_entries
  Py_ssize_t dk_usable;

  // Number of used entries in dk_entries.
  Py_ssize_t dk_nentries;

  dict_lookup_func dk_lookup;
  char dk_indices[];
  /* "PyDictKeyEntry dk_entries[dk_usable];" array follows:
     see the DK_ENTRIES() macro */
};
