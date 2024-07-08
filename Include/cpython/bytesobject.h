typedef struct {
	PyObject_VAR_HEAD
	Py_hash_t ob_shash;
	char ob_sval[1];
	/* Invariants:
	 *   ob_sval contains space for 'ob_size + 1' elements
	 *   ob_sval[ob_size] == 0.
	 *   ob_shash is the hash of the byte string or -1 if not computed yet.
	 */
} PyBytesObject;
