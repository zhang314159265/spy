#pragma once

_Py_IDENTIFIER(little);
_Py_IDENTIFIER(big);

int _PyLong_AsByteArray(PyLongObject *v,
    unsigned char *bytes, size_t n,
    int little_endian, int is_signed) {

  Py_ssize_t i;
  Py_ssize_t ndigits;
  twodigits accum;
  unsigned int accumbits;
  int do_twos_comp;
  digit carry;
  size_t j;
  unsigned char *p;
  int pincr;

  assert(v != NULL && PyLong_Check(v));

  if (Py_SIZE(v) < 0) {
    fail(0);
  } else {
    ndigits = Py_SIZE(v);
    do_twos_comp = 0;
  }

  if (little_endian) {
    p = bytes;
    pincr = 1;
  } else {
    fail(0);
  }

  assert(ndigits == 0 || v->ob_digit[ndigits - 1] != 0);
  j = 0;
  accum = 0;
  accumbits = 0;
  carry = do_twos_comp ? 1 : 0;
  for (i = 0;i < ndigits; ++i) {
    digit thisdigit = v->ob_digit[i];
    if (do_twos_comp) {
      fail(0);
    }
    accum |= (twodigits) thisdigit << accumbits;
    if (i == ndigits - 1) {
      digit s = do_twos_comp ? thisdigit ^ PyLong_MASK : thisdigit;
      while (s != 0) {
        s >>= 1;
        accumbits++;
      }
    } else 
      accumbits += PyLong_SHIFT;

    while (accumbits >= 8) {
      if (j >= n) {
        fail(0);
      }
      ++j;
      *p = (unsigned char)(accum & 0xff);
      p += pincr;
      accumbits -= 8;
      accum >>= 8;
    }
  }

  // Store the straggler (if any).
  assert(accumbits < 8);
  assert(carry == 0);
  if (accumbits > 0) {
    if (j >= n) {
      fail(0);
    }
    ++j;
    if (do_twos_comp) {
      fail(0);
    }
    *p = (unsigned char) (accum & 0xff);
    p += pincr;
  } else if (j == n && n > 0 && is_signed) {
    fail(0);
  }

  {
    unsigned char signbyte = do_twos_comp ? 0xffU : 0U;
    for (; j < n; ++j, p += pincr)
      *p = signbyte;
  }
  return 0;
}

static PyObject *
int_to_bytes_impl(PyObject *self, Py_ssize_t length, PyObject *byteorder,
    int is_signed) {
  int little_endian;
  PyObject *bytes;

  if (_PyUnicode_EqualToASCIIId(byteorder, &PyId_little))
    little_endian = 1;
  else if (_PyUnicode_EqualToASCIIId(byteorder, &PyId_big))
    little_endian = 0;
  else {
    fail(0);
  }

  if (length < 0) {
    fail(0);
  }

  bytes = PyBytes_FromStringAndSize(NULL, length);
  if (bytes == NULL)
    return NULL;

  if (_PyLong_AsByteArray((PyLongObject *) self,
      (unsigned char *) PyBytes_AS_STRING(bytes),
      length, little_endian, is_signed) < 0) {
    Py_DECREF(bytes);
    return NULL;
  }

  return bytes;
}

static PyObject *
int_to_bytes(PyObject *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames) {
  PyObject *return_value = NULL;
  static const char *const _keywords[] = {"length", "byteorder", "signed", NULL};
  static _PyArg_Parser _parser = {NULL, _keywords, "to_bytes", 0};
  PyObject *argsbuf[3];
  Py_ssize_t noptargs = nargs + (kwnames ? PyTuple_GET_SIZE(kwnames) : 0) - 2;
  Py_ssize_t length;
  PyObject *byteorder;
  int is_signed = 0;

  args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser, 2, 2, 0, argsbuf);
  if (!args) {
    fail(0);
  }

  {
    Py_ssize_t ival = -1;
    PyObject *iobj = _PyNumber_Index(args[0]);
    if (iobj != NULL) {
      ival = PyLong_AsSsize_t(iobj);
      Py_DECREF(iobj);
    }
    if (ival == -1 && PyErr_Occurred()) {
      fail(0);
    }
    length = ival;
  }
  if (!PyUnicode_Check(args[1])) {
    fail(0);
  }
  if (PyUnicode_READY(args[1]) == -1) {
    fail(0);
  }
  byteorder = args[1];
  if (!noptargs) {
    goto skip_optional_kwonly;
  }
  is_signed = PyObject_IsTrue(args[2]);
  if (is_signed < 0) {
    fail(0);
  }
skip_optional_kwonly:
  return_value = int_to_bytes_impl(self, length, byteorder, is_signed);

  return return_value;
}


#define INT_TO_BYTES_METHODDEF \
  {"to_bytes", (PyCFunction)(void(*)(void)) int_to_bytes, METH_FASTCALL | METH_KEYWORDS, ""},

#define INT_FROM_BYTES_METHODDEF \
  {"from_bytes", (PyCFunction)(void(*)(void))int_from_bytes, METH_FASTCALL | METH_KEYWORDS | METH_CLASS, ""},

PyObject *
_PyLong_FromByteArray(const unsigned char *bytes, size_t n,
    int little_endian, int is_signed) {

  const unsigned char *pstartbyte;
  int incr;
  const unsigned char *pendbyte;
  size_t numsignificantbytes;
  Py_ssize_t ndigits;
  PyLongObject *v;
  Py_ssize_t idigit = 0;

  if (n == 0) {
    return PyLong_FromLong(0L);
  }

  if (little_endian) {
    pstartbyte = bytes;
    pendbyte = bytes + n - 1;
    incr = 1;
  } else {
    fail(0);
  }

  if (is_signed) {
    is_signed = *pendbyte >= 0x80; 
  }

  {
    size_t i;
    const unsigned char *p = pendbyte;
    const int pincr = -incr; // search MSB to LSB
    const unsigned char insignificant = is_signed ? 0xff : 0x00;

    for (i = 0; i < n; ++i, p += pincr) {
      if (*p != insignificant)
        break;
    }
    numsignificantbytes = n - i;
    if (is_signed && numsignificantbytes < n)
      ++numsignificantbytes;
  }

  if (numsignificantbytes > (PY_SSIZE_T_MAX - PyLong_SHIFT) / 8) {
    fail(0);
  }

  ndigits = (numsignificantbytes * 8 + PyLong_SHIFT - 1) / PyLong_SHIFT;

  v = _PyLong_New(ndigits);
  if (v == NULL)
    return NULL;

  {
    size_t i;
    twodigits carry = 1;
    twodigits accum = 0;
    unsigned int accumbits = 0;

    const unsigned char *p = pstartbyte;

    for (i = 0; i < numsignificantbytes; ++i, p += incr) {
      twodigits thisbyte = *p;
      if (is_signed) {
        fail(0);
      }
      accum |= thisbyte << accumbits;
      accumbits += 8;
      if (accumbits >= PyLong_SHIFT) {
        assert(idigit < ndigits);
        v->ob_digit[idigit] = (digit) (accum & PyLong_MASK);
        ++idigit;
        accum >>= PyLong_SHIFT;
        accumbits -= PyLong_SHIFT;
        assert(accumbits < PyLong_SHIFT);
      }
    }

    assert(accumbits < PyLong_SHIFT);
    if (accumbits) {
      assert(idigit < ndigits);
      v->ob_digit[idigit] = (digit) accum;
      ++idigit;
    }
  }
  
  Py_SET_SIZE(v, is_signed ? -idigit : idigit);
  return (PyObject *) long_normalize(v);

}

static PyObject *
int_from_bytes_impl(PyTypeObject *type, PyObject *bytes_obj,
    PyObject *byteorder, int is_signed) {
  int little_endian;

  PyObject *long_obj, *bytes;

  if (_PyUnicode_EqualToASCIIId(byteorder, &PyId_little))
    little_endian = 1;
  else if (_PyUnicode_EqualToASCIIId(byteorder, &PyId_big))
    little_endian = 0;
  else {
    fail(0);
  }

  bytes = PyObject_Bytes(bytes_obj);
  if (bytes == NULL)
    return NULL;

  long_obj = _PyLong_FromByteArray(
    (unsigned char *) PyBytes_AS_STRING(bytes), Py_SIZE(bytes),
    little_endian, is_signed);
  Py_DECREF(bytes);
  if (long_obj != NULL && type != &PyLong_Type) {
    fail(0);
  }

  return long_obj;
}

static PyObject *
int_from_bytes(PyTypeObject *type, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames) {
  PyObject *return_value = NULL;
  static const char *const _keywords[] = {"bytes", "byteorder", "signed", NULL};
  static _PyArg_Parser _parser = {NULL, _keywords, "from_bytes", 0};
  PyObject *argsbuf[3];
  Py_ssize_t noptargs = nargs + (kwnames ? PyTuple_GET_SIZE(kwnames) : 0) - 2;
  PyObject *bytes_obj;
  PyObject *byteorder;
  int is_signed = 0;

  args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser, 2, 2, 0, argsbuf);
  if (!args) {
    fail(0);
  }
  bytes_obj = args[0];
  if (!PyUnicode_Check(args[1])) {
    fail(0);
  }
  if (PyUnicode_READY(args[1]) == -1) {
    fail(0);
  }
  byteorder = args[1];
  if (!noptargs) {
    goto skip_optional_kwonly;
  }
  is_signed = PyObject_IsTrue(args[2]);
  if (is_signed < 0) {
    fail(0);
  }
skip_optional_kwonly:
  return_value = int_from_bytes_impl(type, bytes_obj, byteorder, is_signed);

  return return_value;
}

static PyMethodDef long_methods[] = {
  INT_TO_BYTES_METHODDEF
  INT_FROM_BYTES_METHODDEF
  {NULL, NULL},
};

static PyObject *
long_new_impl(PyTypeObject *type, PyObject *x, PyObject *obase) {
  Py_ssize_t base;

  if (type != &PyLong_Type) {
    fail(0);
  }
  if (x == NULL) {
    if (obase != NULL) {
      fail(0);
    }
    return PyLong_FromLong(0L);
  }
  if (obase == NULL) 
    return PyNumber_Long(x);
  fail(0);
}

static PyObject *
long_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
  PyObject *return_value = NULL;
  static const char * const _keywords[] = {"", "base", NULL};
  static _PyArg_Parser _parser = {NULL, _keywords, "int", 0};
  PyObject *argsbuf[2];
  PyObject *const *fastargs;
  Py_ssize_t nargs = PyTuple_GET_SIZE(args);
  Py_ssize_t noptargs = nargs + (kwargs ? PyDict_GET_SIZE(kwargs) : 0) - 0;
  PyObject *x = NULL;
  PyObject *obase = NULL;

  fastargs = _PyArg_UnpackKeywords(_PyTuple_CAST(args)->ob_item, nargs, kwargs, NULL, &_parser, 0, 2, 0, argsbuf);
  if (!fastargs) {
    fail(0);
  }
  if (nargs < 1) {
    goto skip_optional_posonly;
  }
  noptargs--;
  x = fastargs[0];
skip_optional_posonly:
  if (!noptargs) {
    goto skip_optional_pos;
  }
  obase = fastargs[1];
skip_optional_pos:
  return_value = long_new_impl(type, x, obase);
exit:
  return return_value;
}

// defined in cpy/Objects/longobject.c
PyTypeObject PyLong_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "int",
  .tp_basicsize = offsetof(PyLongObject, ob_digit),
  .tp_flags = Py_TPFLAGS_LONG_SUBCLASS,
  .tp_free = PyObject_Del,
  .tp_hash = (hashfunc) long_hash,
  .tp_richcompare = long_richcompare,
  .tp_as_number = &long_as_number,
  .tp_repr = long_to_decimal_string,
  .tp_methods = long_methods,
  .tp_new = long_new,
};


long PyLong_AsLongAndOverflow(PyObject *vv, int *overflow) {
  PyLongObject *v;
  int do_decref = 0;
  long res;
  Py_ssize_t i;

  *overflow = 0;
  if (vv == NULL) {
    assert(false);
  }

  if (PyLong_Check(vv)) {
    v = (PyLongObject *) vv;
  } else {
    v = (PyLongObject *) _PyNumber_Index(vv);
    if (v == NULL)
      return -1;
    do_decref = 1;
  }

  res = -1;
  i = Py_SIZE(v);
  switch (i) {
  case -1:
    res = -(sdigit) v->ob_digit[0];
    break;
  case 0:
    res = 0;
    break;
  case 1:
    res = v->ob_digit[0];
    break;
  default:
    assert(false);
  }

  if (do_decref) {
    Py_DECREF(v);
  }
  return res;
}

