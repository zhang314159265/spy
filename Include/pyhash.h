#pragma once

#define _PyHASH_BITS 61
#define _PyHASH_MODULUS (((size_t) 1 << _PyHASH_BITS) - 1)

Py_hash_t _Py_HashBytes(const void *src, Py_ssize_t len) {
	Py_hash_t x;
	if (len == 0) {
		return 0;
	}
#if 0
	x = PyHash_Func.hash(src, len);
#else
	// a dummy hash implementation
	x = 0;
	for (int i = 0; i < len; ++i) {
		x ^= ((uint8_t*) src)[i];
	}
#endif
	if (x == -1) {
		return -2;
	}
	return x;
}

Py_hash_t
_Py_HashPointerRaw(const void *p) {
	size_t y = (size_t) p;
	y = (y >> 4) | (y << (8 * SIZEOF_VOID_P - 4));
	return (Py_hash_t) y;
}

Py_hash_t
_Py_HashPointer(const void *p) {
	Py_hash_t x = _Py_HashPointerRaw(p);
	if (x == -1) {
		x = -2;
	}
	return x;
}

Py_hash_t
_Py_HashDouble(PyObject *inst, double v) {
	// TODO follow cpy
	return _Py_HashBytes(&v, sizeof(double));
}
