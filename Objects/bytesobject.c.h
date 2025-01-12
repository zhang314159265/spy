#pragma once

static PyObject *bytes_concat(PyObject *a, PyObject *b) {
  Py_buffer va, vb;
  PyObject *result = NULL;

  va.len = vb.len = -1;

  if (PyObject_GetBuffer(a, &va, PyBUF_SIMPLE) != 0 ||
    PyObject_GetBuffer(b, &vb, PyBUF_SIMPLE) != 0) {
    fail(0);
  }
  if (va.len == 0 && PyBytes_CheckExact(b)) {
    fail(0);
  }
  if (vb.len == 0 && PyBytes_CheckExact(a)) {
    fail(0);
  }
  if (va.len > PY_SSIZE_T_MAX - vb.len) {
    fail(0);
  }
  result = PyBytes_FromStringAndSize(NULL, va.len + vb.len);
  if (result != NULL) {
    memcpy(PyBytes_AS_STRING(result), va.buf, va.len);
    memcpy(PyBytes_AS_STRING(result) + va.len, vb.buf, vb.len);
  }

  if (va.len != -1)
    PyBuffer_Release(&va);
  if (vb.len != -1)
    PyBuffer_Release(&vb);
  return result;
}

PyObject *
PyBytes_Repr(PyObject *obj, int smartquotes) {
  PyBytesObject* op = (PyBytesObject *) obj;
  Py_ssize_t i, length = Py_SIZE(op);
  Py_ssize_t newsize, squotes, dquotes;
  PyObject *v;
  unsigned char quote;
  const unsigned char *s;
  Py_UCS1 *p;

  // Compute size of output string
  squotes = dquotes = 0;
  newsize = 3; // b''
  s = (const unsigned char *) op->ob_sval;
  for (i = 0; i < length; i++) {
    Py_ssize_t incr = 1;
    switch (s[i]) {
    case '\'': squotes++; break;
    case '\"': dquotes++; break;
    case '\\': case '\t': case '\n': case '\r':
      incr = 2; break;
    default:
      if (s[i] < ' ' || s[i] >= 0x7f)
        incr = 4; // \xHH
    }
    if (newsize > PY_SSIZE_T_MAX - incr)
      fail(0);
    newsize += incr;
  }
  quote = '\'';
  if (smartquotes && squotes && !dquotes)
    quote = '"';
  if (squotes && quote == '\'') {
    newsize += squotes;
  }

  v = PyUnicode_New(newsize, 127);
  if (v == NULL)
    return NULL;
  p = PyUnicode_1BYTE_DATA(v);

  *p++ = 'b';
  *p++ = quote;

  for (i = 0; i < length; i++) {
    unsigned char c = op->ob_sval[i];
    if (c == quote || c == '\\')
      *p++ = '\\', *p++ = c;
    else if (c == '\t')
      *p++ = '\\', *p++ = 't';
    else if (c == '\n') 
      *p++ = '\\', *p++ = 'n';
    else if (c == '\r') 
      *p++ = '\\', *p++ = 'r';
    else if (c < ' ' || c >= 0x7f) {
      fail(0);
    } else {
      *p++ = c;
    }
  }
  *p++ = quote;
  return v;
}

static PyObject *
bytes_repr(PyObject *op) {
  return PyBytes_Repr(op, 1);
}

// defined in cpy/Objects/bytesobject.c
PyTypeObject PyBytes_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "bytes",
  .tp_basicsize = PyBytesObject_SIZE,
	.tp_flags = Py_TPFLAGS_BYTES_SUBCLASS,
	.tp_hash = (hashfunc) bytes_hash,
	.tp_free = PyObject_Del,
  .tp_as_sequence = &bytes_as_sequence,
  .tp_as_buffer = &bytes_as_buffer,
  .tp_repr = (reprfunc) bytes_repr,
};


