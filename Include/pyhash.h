#pragma once

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
