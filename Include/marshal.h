#pragma once

#define FLAG_REF '\x80'

#define TYPE_CODE 'c'
#define TYPE_NONE 'N'
#define TYPE_STRING 's'
#define TYPE_TUPLE '('
#define TYPE_SMALL_TUPLE ')'
#define TYPE_ASCII 'a'
#define TYPE_SHORT_ASCII 'z'
#define TYPE_SHORT_ASCII_INTERNED 'Z'
#define TYPE_REF 'r'
#define TYPE_INT 'i'
#define TYPE_TRUE 'T'
#define TYPE_FALSE 'F'
#define TYPE_LONG 'l'
#define TYPE_SET '<'
#define TYPE_FROZENSET '>'

#define SIZE32_MAX 0x7FFFFFFF

#define MAX_MARSHAL_STACK_DEPTH 1000

#define PyLong_MARSHAL_SHIFT 15
#define PyLong_MARSHAL_BASE ((short) 1 << PyLong_MARSHAL_SHIFT)
#define PyLong_MARSHAL_RATIO (PyLong_SHIFT / PyLong_MARSHAL_SHIFT)

typedef struct {
	FILE *fp;
  int depth;

	const char *ptr;
	const char *end;
  char *buf;
	PyObject *refs; // a list
} RFILE;

static int
r_byte(RFILE *p) {
	int c = EOF;

	if (p->ptr != NULL) {
		if (p->ptr < p->end)
			c = (unsigned char) *p->ptr++;
		return c;
	}
	assert(false);
}

static Py_ssize_t
r_ref_reserve(int flag, RFILE *p) {
  if (flag) {
    assert(false);
  } else {
    return 0;
  }
}

static const char *
r_string(Py_ssize_t n, RFILE *p) {
  if (p->ptr != NULL) {
    const char *res = p->ptr;
    Py_ssize_t left = p->end - p->ptr;
    if (left < n) {
      assert(false);
    }
    p->ptr += n;
    return res;
  }
  assert(false);
}

static int
r_short(RFILE *p) {
  short x = -1;
  const unsigned char *buffer;

  buffer = (const unsigned char *) r_string(2, p);
  if (buffer != NULL) {
    x = buffer[0];
    x |= buffer[1] << 8;
    x |= -(x & 0x8000);
  }
  return x;
}

static long
r_long(RFILE *p) {
  long x = -1;
  const unsigned char *buffer;

  buffer = (const unsigned char *) r_string(4, p);
  if (buffer != NULL) {
    x = buffer[0];
    x |= (long) buffer[1] << 8;
    x |= (long) buffer[2] << 16;
    x |= (long) buffer[3] << 24;
#if SIZEOF_LONG > 4
    x |= -(x & 0x80000000L);
#endif
  }
  return x;
}

static PyObject *
r_ref(PyObject *o, int flag, RFILE *p) {
  assert(flag & FLAG_REF);

  if (o == NULL)
    return NULL;
  if (PyList_Append(p->refs, o) < 0) {
    Py_DECREF(o);
    return NULL;
  }
  return o;
}

static PyObject *
r_ref_insert(PyObject *o, Py_ssize_t idx, int flag, RFILE *p) {
  if (o != NULL && flag) {
    PyObject *tmp = PyList_GET_ITEM(p->refs, idx);
    Py_INCREF(o);
    PyList_SET_ITEM(p->refs, idx, o);
    Py_DECREF(tmp);
  }
  return o;
}

static PyObject *
r_PyLong(RFILE *p) {
  PyLongObject *ob;
  long n, size, i;
  int j, md, shorts_in_top_digit;
  digit d;

  n = r_long(p);
  if (PyErr_Occurred())
    return NULL;

  if (n == 0)
    return (PyObject *) _PyLong_New(0);

  if (n < -SIZE32_MAX || n > SIZE32_MAX) {
    fail(0);
  }

  size = 1 + (Py_ABS(n) - 1) / PyLong_MARSHAL_RATIO;
  shorts_in_top_digit = 1 + (Py_ABS(n) - 1) % PyLong_MARSHAL_RATIO;
  ob = _PyLong_New(size);
  if (ob == NULL)
    return NULL;

  Py_SET_SIZE(ob, n > 0 ? size : -size);

  for (i = 0; i < size - 1; i++) {
    d = 0;
    for (j = 0; j < PyLong_MARSHAL_RATIO; j++) {
      md = r_short(p);
      if (PyErr_Occurred()) {
        fail(0);
      }
      if (md < 0 || md > PyLong_MARSHAL_BASE)
        fail(0);
      d += (digit) md << j * PyLong_MARSHAL_SHIFT;
    }
    ob->ob_digit[i] = d;
  }

  d = 0;
  for (j = 0; j < shorts_in_top_digit; j++) {
    md = r_short(p);
    if (PyErr_Occurred()) {
      fail(0);
    }
    if (md < 0 || md > PyLong_MARSHAL_BASE) {
      fail(0);
    }
    if (md == 0 && j == shorts_in_top_digit - 1) {
      fail(0);
    }
    d += (digit) md << j * PyLong_MARSHAL_SHIFT;
  }
  if (PyErr_Occurred()) {
    fail(0);
  }
  ob->ob_digit[size - 1] = d;
  return (PyObject *) ob;
}

static PyObject *
r_object(RFILE *p) {
	PyObject *v, *v2;
  Py_ssize_t idx = 0;
  long i, n;
	int type, code = r_byte(p);
	int flag, is_interned = 0;
	PyObject *retval = NULL;

  p->depth++;

  if (p->depth > MAX_MARSHAL_STACK_DEPTH) {
    assert(false);
  }

	flag = code & FLAG_REF;
	type = code & ~FLAG_REF;

#define R_REF(O) do { \
  if (flag) \
    O = r_ref(O, flag, p); \
} while(0)

  // printf("r_object type is %d ('%c')\n", type, (char)type);

	switch (type) {
	case TYPE_CODE:
		{
      int argcount;
      int posonlyargcount;
      int kwonlyargcount;
      int nlocals;
      int stacksize;
      int flags;
      PyObject *code = NULL;
      PyObject *consts = NULL;
      PyObject *names = NULL;
      PyObject *varnames = NULL;
      PyObject *freevars = NULL;
      PyObject *cellvars = NULL;
      PyObject *filename = NULL;
      PyObject *name = NULL;
      int firstlineno;
      PyObject *linetable = NULL;

      idx = r_ref_reserve(flag, p);
      if (idx < 0)  
        break;

      v = NULL;

      argcount = (int) r_long(p);
      if (PyErr_Occurred())
        goto code_error;
      posonlyargcount = (int) r_long(p);
      if (PyErr_Occurred()) {
        goto code_error;
      }
      kwonlyargcount = (int) r_long(p);
      if (PyErr_Occurred())
        goto code_error;
      nlocals = (int) r_long(p);
      if (PyErr_Occurred())
        goto code_error;
      stacksize = (int) r_long(p);
      if (PyErr_Occurred())
        goto code_error;
      flags = (int) r_long(p);
      if (PyErr_Occurred())
        goto code_error;
      code = r_object(p);
      if (code == NULL)
        goto code_error;
      consts = r_object(p);
      if (consts == NULL)
        goto code_error;
      names = r_object(p);
      if (names == NULL)
        goto code_error;
      varnames = r_object(p);
      if (varnames == NULL)
        goto code_error;
      freevars = r_object(p);
      if (freevars == NULL)
        goto code_error;
      cellvars = r_object(p);
      if (cellvars == NULL)
        goto code_error;
      filename = r_object(p);
      if (filename == NULL)
        goto code_error;
      name = r_object(p);
      if (name == NULL)
        goto code_error;
      firstlineno = (int) r_long(p);
      if (firstlineno == -1 && PyErr_Occurred())
        break;
      linetable = r_object(p);
      if (linetable == NULL)
        goto code_error;
      v = (PyObject *) PyCode_NewWithPosOnlyArgs(
        argcount, posonlyargcount, kwonlyargcount,
        nlocals, stacksize, flags,
        code, consts, names, varnames,
        freevars, cellvars, /* filename, */ name,
        firstlineno, linetable);
      v = r_ref_insert(v, idx, flag, p);
    code_error:
      Py_XDECREF(code);
      Py_XDECREF(consts);
      Py_XDECREF(names);
      Py_XDECREF(varnames);
      Py_XDECREF(freevars);
      Py_XDECREF(cellvars);
      Py_XDECREF(filename);
      Py_XDECREF(name);
      Py_XDECREF(linetable);
		}
		retval = v;
		break;
  case TYPE_STRING: {
    const char *ptr;
    n = r_long(p);
    if (PyErr_Occurred())
      break;
    if (n < 0 || n > SIZE32_MAX) {
      assert(false);
    }
    v = PyBytes_FromStringAndSize((char *) NULL, n);
    if (v == NULL)
      break;
    ptr = r_string(n, p);
    if (ptr == NULL) {
      Py_DECREF(v);
      break;
    }
    memcpy(PyBytes_AS_STRING(v), ptr, n);
    retval = v;
    R_REF(retval);
    break;
  }
  case TYPE_SMALL_TUPLE: {
    n = (unsigned char) r_byte(p);
    if (PyErr_Occurred())
      break;
    goto _read_tuple;
  }
  case TYPE_TUPLE:
    assert(false);
  _read_tuple:
    v = PyTuple_New(n);
    R_REF(v);
    if (v == NULL)
      break;
    for (i = 0; i < n; i++) {
      v2 = r_object(p);
      if (v2 == NULL) {
        assert(false);
      }
      PyTuple_SET_ITEM(v, i, v2);
    }
    retval = v;
    break;
  case TYPE_ASCII: {
    n = r_long(p);
    if (PyErr_Occurred())
      break;
    if (n < 0 || n > SIZE32_MAX) {
      assert(false);
    }
    goto _read_ascii;
  }
  case TYPE_SHORT_ASCII_INTERNED:
    is_interned = 1;
  case TYPE_SHORT_ASCII:
    n = r_byte(p);
    if (n == EOF) {
      assert(false);
    }
  _read_ascii: {
    const char *ptr;
    ptr = r_string(n, p);
    if (ptr == NULL)
      break;
    v = PyUnicode_FromKindAndData(PyUnicode_1BYTE_KIND, ptr, n);
    if (v == NULL)
      break;
    if (is_interned) {
      PyUnicode_InternInPlace(&v);
    }
    retval = v;
    R_REF(retval);
    break;
  }
  case TYPE_NONE: {
    Py_INCREF(Py_None);
    retval = Py_None;
    break;
  }
  case TYPE_REF: {
    n = r_long(p);
    if (n < 0 || n >= PyList_GET_SIZE(p->refs)) {
      assert(false);
    }
    v = PyList_GET_ITEM(p->refs, n);
    if (v == Py_None) {
      assert(false);
    }
    Py_INCREF(v);
    retval = v;
    break;
  }
  case TYPE_INT:
    n = r_long(p);
    retval = PyErr_Occurred() ? NULL : PyLong_FromLong(n);
    R_REF(retval);
    break;
  case TYPE_TRUE:
    Py_INCREF(Py_True);
    retval = Py_True;
    break;
  case TYPE_FALSE:
    Py_INCREF(Py_False);
    retval = Py_False;
    break;
  case TYPE_LONG:
    retval = r_PyLong(p);
    R_REF(retval);
    break;
  case TYPE_SET:
  case TYPE_FROZENSET:
    n = r_long(p);
    if (PyErr_Occurred())
      break;
    if (n < 0 || n > SIZE32_MAX) {
      fail(0);
    }
    if (n == 0 && type == TYPE_FROZENSET) {
      fail(0);
    } else {
      v = (type == TYPE_SET) ? PySet_New(NULL) : PyFrozenSet_New(NULL);
      if (type == TYPE_SET) {
        fail(0);
      } else {
        idx = r_ref_reserve(flag, p);
        if (idx < 0)
          Py_CLEAR(v);
      }
      if (v == NULL)
        break;

      for (i = 0; i < n; i++) {
        v2 = r_object(p);
        if (v2 == NULL) {
          fail(0);
        }
        if (PySet_Add(v, v2) == -1) {
          fail(0);
        }
        Py_DECREF(v2);
      }
      if (type != TYPE_SET) {
        v = r_ref_insert(v, idx, flag, p);
      }
      retval = v;
    }
    break;
	default:
		printf("r_object unsupported type is %d ('%c')\n", type, (char)type);
		assert(false);
	}
  p->depth--;
  return retval;
}

static PyObject *
read_object(RFILE *p) {
	PyObject *v;

	v = r_object(p);
	return v;
}

PyObject *
PyMarshal_ReadObjectFromString(const char *str, Py_ssize_t len) {
	RFILE rf;
	PyObject *result;

	rf.fp = NULL;
	rf.ptr = str;
	rf.end = str + len;
  rf.buf = NULL;
  rf.depth = 0;
	rf.refs = PyList_New(0);
	if (rf.refs == NULL)
		return NULL;
	result = read_object(&rf);
  Py_DECREF(rf.refs);
  if (rf.buf != NULL)
    PyMem_Free(rf.buf);
  return result;
}
