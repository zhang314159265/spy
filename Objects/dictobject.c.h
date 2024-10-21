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

static PyObject *dict_iter(PyDictObject *dict) {
	assert(false);
}
