#pragma once

#if PYLONG_BITS_IN_DIGIT == 30
typedef uint32_t digit;
typedef int32_t sdigit; // signed variant of digit
typedef uint64_t twodigits;
typedef int64_t stwodigits;
#define PyLong_SHIFT 30
#define _PyLong_DECIMAL_SHIFT 9
#define _PyLong_DECIMAL_BASE ((digit) 1000000000)
#elif PYLONG_BITS_IN_DIGIT == 15
#error PYLONG_BITS_IN_DIGIT == 15 not supported
typedef unsigned short digit;
typedef short sdigit;
#define PyLong_SHIFT 15
#else
#error "PYLONG_BITS_IN_DIGIT should be 15 or 30"
#endif

#define PyLong_BASE ((digit) 1 << PyLong_SHIFT)
#define PyLong_MASK ((digit) (PyLong_BASE - 1))

#define FIVEARY_CUTOFF 8

struct _longobject {
  PyObject_VAR_HEAD
  digit ob_digit[1];
};

static Py_hash_t long_hash(PyLongObject *v);
static PyObject *long_richcompare(PyObject *self, PyObject *other, int op);
static PyObject *long_add(PyLongObject *a, PyLongObject *b);
static PyObject *long_sub(PyLongObject *a, PyLongObject *b);
static PyObject *long_div(PyObject *a, PyObject *b);
static PyObject *long_mod(PyObject *a, PyObject *b);
static PyObject *long_true_divide(PyObject *v, PyObject *w);
static PyObject *long_pow(PyObject *v, PyObject *w, PyObject *x);
static PyObject *long_mul(PyLongObject *a, PyLongObject *b);
static PyObject *long_to_decimal_string(PyObject *aa);
static PyObject *long_and(PyObject *a, PyObject *b);
static PyObject *long_or(PyObject *a, PyObject *b);
static PyObject *long_xor(PyObject *a, PyObject *b);
static PyObject *long_lshift(PyObject *a, PyObject *b);
static PyObject *long_rshift(PyObject *a, PyObject *b);
static PyObject *long_neg(PyLongObject *v);
static PyObject *long_invert(PyLongObject *v);
static PyObject *long_long(PyObject *v);
static int long_bool(PyLongObject *v) { return Py_SIZE(v) != 0; }

static PyNumberMethods long_as_number = {
  .nb_inplace_add = 0,
  .nb_add = (binaryfunc) long_add,
  .nb_subtract = (binaryfunc) long_sub,
  .nb_floor_divide = long_div,
  .nb_remainder = long_mod,
  .nb_true_divide = long_true_divide,
  .nb_power = long_pow,
  .nb_multiply = (binaryfunc) long_mul,
	.nb_and = long_and,
  .nb_or = long_or,
  .nb_xor = long_xor,
  .nb_lshift = long_lshift,
  .nb_rshift = long_rshift,
  .nb_negative = (unaryfunc) long_neg,
  .nb_positive = long_long,
  .nb_invert = (unaryfunc) long_invert,
  .nb_bool = (inquiry) long_bool,
  .nb_index = long_long,
};

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
};

// defined in cpy/Objects/longobject.c
PyLongObject * _PyLong_New(Py_ssize_t size) {
  PyLongObject * result;
  result = PyObject_Malloc(offsetof(PyLongObject, ob_digit) +
    size * sizeof(digit));
  if (!result) {
    assert(false);
  }
  _PyObject_InitVar((PyVarObject*) result, &PyLong_Type, size);
  return result;
}

static Py_hash_t
long_hash(PyLongObject *v) {
  Py_uhash_t x;
  Py_ssize_t i;
  int sign;

  i = Py_SIZE(v);
  switch (i) {
  case -1: return v->ob_digit[0] == 1 ? -2 : -(sdigit)v->ob_digit[0];
  case 0: return 0;
  case 1: return v->ob_digit[0];
  }
  sign = 1;
  x = 0;
  if (i < 0) {
    sign = -1;
    i = -(i);
  }
  while (--i >= 0) {
    x = ((x << PyLong_SHIFT) & _PyHASH_MODULUS) |
      (x >> (_PyHASH_BITS - PyLong_SHIFT));
    x += v->ob_digit[i];
    if (x >= _PyHASH_MODULUS)
      x -= _PyHASH_MODULUS;
  }
  x = x * sign;
  if (x == (Py_uhash_t) -1)
    x = (Py_uhash_t) -2;
  return (Py_hash_t) x;
}

#define CHECK_BINOP(v, w) \
  do { \
    if (!PyLong_Check(v) || !PyLong_Check(w)) \
      Py_RETURN_NOTIMPLEMENTED; \
  } while (0)

#include "boolobject.h"

static Py_ssize_t
long_compare(PyLongObject *a, PyLongObject *b)
{
  Py_ssize_t sign = Py_SIZE(a) - Py_SIZE(b);
  if (sign == 0) {
    Py_ssize_t i = Py_ABS(Py_SIZE(a));
    sdigit diff = 0;
    while (--i >= 0) {
      diff = (sdigit) a->ob_digit[i] - (sdigit) b->ob_digit[i];
      if (diff) {
        break;
      }
    }
    sign = Py_SIZE(a) < 0 ? -diff : diff;
  }
  return sign;
}

static PyObject *long_richcompare(PyObject *self, PyObject *other, int op) {
  Py_ssize_t result;
  CHECK_BINOP(self, other);
  if (self == other)
    result = 0;
  else
    result = long_compare((PyLongObject *) self, (PyLongObject *) other);
  Py_RETURN_RICHCOMPARE(result, 0, op);
}

#define MEDIUM_VALUE(x) (assert(-1 <= Py_SIZE(x) && Py_SIZE(x) <= 1), \
  Py_SIZE(x) < 0 ? -(sdigit)(x)->ob_digit[0] : \
  (Py_SIZE(x) == 0 ? (sdigit) 0 : \
  (sdigit) (x)->ob_digit[0]))

static PyObject *long_add(PyLongObject *a, PyLongObject *b) {
  if (Py_ABS(Py_SIZE(a)) <= 1 && Py_ABS(Py_SIZE(b)) <= 1) {
    return PyLong_FromLong(MEDIUM_VALUE(a) + MEDIUM_VALUE(b));
  }
  assert(false);
}

static PyObject *long_sub(PyLongObject *a, PyLongObject *b) {
  if (Py_ABS(Py_SIZE(a)) <= 1 && Py_ABS(Py_SIZE(b)) <= 1) {
    return PyLong_FromLong(MEDIUM_VALUE(a) - MEDIUM_VALUE(b));
  }
  assert(false);
}

static PyObject *
fast_floor_div(PyLongObject *a, PyLongObject *b) {
  sdigit left = a->ob_digit[0];
  sdigit right = b->ob_digit[0];
  sdigit div;

  assert(Py_ABS(Py_SIZE(a)) == 1);
  assert(Py_ABS(Py_SIZE(b)) == 1);

  if (Py_SIZE(a) == Py_SIZE(b)) {
    // 'a' and 'b' have the same sign
    div = left / right;
  } else {
    assert(false);
  }

  return PyLong_FromLong(div);
}

static PyObject *long_div(PyObject *a, PyObject *b) {
  if (Py_ABS(Py_SIZE(a)) == 1 && Py_ABS(Py_SIZE(b)) == 1) {
    return fast_floor_div((PyLongObject *) a, (PyLongObject *) b);
  }
  assert(false);
}

static PyObject *
long_long(PyObject *v) {
  if (PyLong_CheckExact(v))
    Py_INCREF(v);
  else
    assert(false);
  return v;
}

static inline PyObject *_PyLong_GetZero(void);

static int
long_divrem(PyLongObject *a, PyLongObject *b, PyLongObject **pdiv, PyLongObject **prem) {
  Py_ssize_t size_a = Py_ABS(Py_SIZE(a)), size_b = Py_ABS(Py_SIZE(b));

  if (size_b == 0) {
    // div by 0
    assert(false);
  }

  if (size_a < size_b ||
      (size_a == size_b &&
        a->ob_digit[size_a - 1] < b->ob_digit[size_b - 1])) {
    // |a| < |b|
    *prem = (PyLongObject *) long_long((PyObject *) a);
    if (*prem == NULL) {
      return -1;
    }
    PyObject *zero = _PyLong_GetZero();
    Py_INCREF(zero);
    *pdiv = (PyLongObject *) zero;
    return 0;
  }
  assert(false);
}

static int
l_divmod(PyLongObject *v, PyLongObject *w,
    PyLongObject **pdiv, PyLongObject **pmod) {
  PyLongObject *div, *mod;
  if (Py_ABS(Py_SIZE(v)) == 1 && Py_ABS(Py_SIZE(w)) == 1) {
    assert(false);
  }
  if (long_divrem(v, w, &div, &mod) < 0)
    return -1;
  if ((Py_SIZE(mod) < 0 && Py_SIZE(w) > 0) ||
      (Py_SIZE(mod) > 0 && Py_SIZE(w) < 0)) {
    assert(false);
  }
  if (pdiv != NULL)
    *pdiv = div;
  else
    Py_DECREF(div);

  if (pmod != NULL)
    *pmod = mod;
  else
    Py_DECREF(mod);

  return 0;
}

static PyObject *
fast_mod(PyLongObject *a, PyLongObject *b) {
  sdigit left = a->ob_digit[0];
  sdigit right = b->ob_digit[0];
  sdigit mod;

  assert(Py_ABS(Py_SIZE(a)) == 1);
  assert(Py_ABS(Py_SIZE(b)) == 1);

  if (Py_SIZE(a) == Py_SIZE(b)) {
    mod = left % right;
  } else {
    assert(false);
  }
  
  return PyLong_FromLong(mod * (sdigit) Py_SIZE(b));
}

PyObject *
PyLong_FromLongLong(long long ival);

static PyObject *
long_mul(PyLongObject *a, PyLongObject *b) {
  PyLongObject *z;

  CHECK_BINOP(a, b);

  if (Py_ABS(Py_SIZE(a)) <= 1 && Py_ABS(Py_SIZE(b)) <= 1) {
    stwodigits v = (stwodigits)(MEDIUM_VALUE(a)) * MEDIUM_VALUE(b);
    return PyLong_FromLongLong((long long) v);
  }
  assert(false);
}

static PyObject *long_mod(PyObject *a, PyObject *b) {
  PyLongObject *mod;

  // printf("abs size for mod: %d v.s. %d\n", Py_ABS(Py_SIZE(a)), Py_ABS(Py_SIZE(b)));
  if (Py_ABS(Py_SIZE(a)) == 1 && Py_ABS(Py_SIZE(b)) == 1) {
    return fast_mod((PyLongObject *) a, (PyLongObject *) b);
  }

  if (l_divmod((PyLongObject *) a, (PyLongObject *) b, NULL, &mod) < 0) {
    mod = NULL;
  }
  return (PyObject *) mod;
}

static PyObject *long_true_divide(PyObject *v, PyObject *w) {
  CHECK_BINOP(v, w);
  assert(false);
}

static PyObject *long_pow(PyObject *v, PyObject *w, PyObject *x) {
  PyLongObject *a, *b, *c;
  int negativeOutput = 0;

  PyLongObject *z = NULL;
  Py_ssize_t i, j, k;
  PyLongObject *temp = NULL;
  
  PyLongObject *table[32] = {0};

  CHECK_BINOP(v, w);
  a = (PyLongObject *) v; Py_INCREF(a);
  b = (PyLongObject *) w; Py_INCREF(b);
  if (PyLong_Check(x)) {
    assert(false);
  } else if (x == Py_None)
    c = NULL;
  else {
    Py_DECREF(a);
    Py_DECREF(b);
    Py_RETURN_NOTIMPLEMENTED;
  }

  if (Py_SIZE(b) < 0 && c == NULL) {
    assert(false);
  }
  if (c) {
    assert(false);
  }

  z = (PyLongObject *) PyLong_FromLong(1L);
  if (z == NULL)
    assert(false);

#define REDUCE(X) \
  do { \
    if (c != NULL) { \
      assert(false); \
    } \
  } while(0)

#define MULT(X, Y, result) \
  do { \
    temp = (PyLongObject *) long_mul(X, Y); \
    if (temp == NULL) \
      assert(false); \
    Py_XDECREF(result); \
    result = temp; \
    temp = NULL; \
    REDUCE(result); \
  } while(0)

  if (Py_SIZE(b) <= FIVEARY_CUTOFF) {
    for (i = Py_SIZE(b) - 1; i >= 0; --i) {
      digit bi = b->ob_digit[i];

      for (j = (digit) 1 << (PyLong_SHIFT - 1); j != 0; j >>= 1) {
        MULT(z, z, z);
        if (bi & j)
          MULT(z, a, z);
      }
    }
  } else {
    assert(false);
  }

  if (negativeOutput && (Py_SIZE(z) != 0)) {
    assert(false);
  }

  goto Done;
Error:
  Py_CLEAR(z);
Done:
  if (Py_SIZE(b) > FIVEARY_CUTOFF) {
    for (i = 0; i < 32; ++i) {
      Py_XDECREF(table[i]);
    }
  }
  Py_DECREF(a);
  Py_DECREF(b);
  Py_XDECREF(c);
  Py_XDECREF(temp);
  return (PyObject *) z;
}

static int
long_to_decimal_string_internal(PyObject *aa,
		PyObject **p_output,
		_PyUnicodeWriter *writer,
		_PyBytesWriter *bytes_writer,
		char **bytes_str);

static PyObject *long_to_decimal_string(PyObject *aa) {
  PyObject *v;
  if (long_to_decimal_string_internal(aa, &v, NULL, NULL, NULL) == -1)
    return NULL;
  return v;
}

static PyLongObject *
long_normalize(PyLongObject *v) {
	Py_ssize_t j = Py_ABS(Py_SIZE(v));
	Py_ssize_t i = j;

	while (i > 0 && v->ob_digit[i - 1] == 0)
		--i;
	if (i != j) {
		Py_SET_SIZE(v, (Py_SIZE(v) < 0) ? -(i) : i);
	}
	return v;
}

static PyLongObject *
maybe_small_long(PyLongObject *v) {
	// TODO follow cpy
	return v;
}

static PyObject *
long_bitwise(PyLongObject *a,
		char op,
		PyLongObject *b) {
	int nega, negb, negz;
	Py_ssize_t size_a, size_b, size_z, i;
	PyLongObject *z;

	size_a = Py_ABS(Py_SIZE(a));
	nega = Py_SIZE(a) < 0;
	if (nega) {
		assert(false);
	} else
		Py_INCREF(a);

	size_b = Py_ABS(Py_SIZE(b));
	negb = Py_SIZE(b) < 0;
	if (negb) {
		assert(false);
	} else
		Py_INCREF(b);

	if (size_a < size_b) {
		z = a; a = b; b = z;
		size_z = size_a; size_a = size_b; size_b = size_z;
		negz = nega; nega = negb; negb = negz;
	}
	
	switch (op){
  case '^':
    negz = nega ^ negb;
    size_z = size_a;
    break;
	case '&':
		negz = nega & negb;
		size_z = negb ? size_a : size_b;
		break;
  case '|':
    negz = nega | negb;
    size_z = negb ? size_b : size_a;
    break;
	default:
		assert(false);
	}
	z = _PyLong_New(size_z + negz);
	if (z == NULL) {
		Py_DECREF(a);
		Py_DECREF(b);
		return NULL;
	}

	switch (op) {
	case '&':
		for (i = 0; i < size_b; ++i) {
			z->ob_digit[i] = a->ob_digit[i] & b->ob_digit[i];
		}
		break;
  case '|':
    for (i = 0; i < size_b; ++i) {
			z->ob_digit[i] = a->ob_digit[i] | b->ob_digit[i];
    }
    break;
  case '^':
    for (i = 0; i < size_b; ++i) {
			z->ob_digit[i] = a->ob_digit[i] ^ b->ob_digit[i];
    }
    break;
	default:
		assert(false);
	}
	if (op == '^' && negb) {
		assert(false);
	} else if (i < size_z) {
		assert(false);
	}

	if (negz) {
		assert(false);
	}

	Py_DECREF(a);
	Py_DECREF(b);

	return (PyObject *)maybe_small_long(long_normalize(z));
}

static PyObject *long_and(PyObject *a, PyObject *b) {
	PyObject *c;
	CHECK_BINOP(a, b);
	c = long_bitwise((PyLongObject *)a, '&', (PyLongObject*) b);
	return c;
}

static PyObject *long_or(PyObject *a, PyObject *b) {
  PyObject *c;
  CHECK_BINOP(a, b);
  c = long_bitwise((PyLongObject *) a, '|', (PyLongObject *) b);
  return c;
}

static PyObject *long_xor(PyObject *a, PyObject *b) {
  PyObject *c;
  CHECK_BINOP(a, b);
  c = long_bitwise((PyLongObject *) a, '^', (PyLongObject *) b);
  return c;
}

Py_ssize_t
PyLong_AsSsize_t(PyObject *vv) {
  PyLongObject *v;
  Py_ssize_t i;

  if (vv == NULL) {
    assert(false);
  }
  if (!PyLong_Check(vv)) {
    assert(false);
  }

  v = (PyLongObject *) vv;
  i = Py_SIZE(v);
  switch (i) {
  case -1: return -(sdigit)v->ob_digit[0];
  case 0: return 0;
  case 1: return v->ob_digit[0];
  }
  assert(false);
}

static int
divmod_shift(PyObject *shiftby, Py_ssize_t *wordshift, digit *remshift) {
  assert(PyLong_Check(shiftby));
  assert(Py_SIZE(shiftby) >= 0);
  Py_ssize_t lshiftby = PyLong_AsSsize_t((PyObject *) shiftby);
  if (lshiftby >= 0) {
    *wordshift = lshiftby / PyLong_SHIFT;
    *remshift = lshiftby % PyLong_SHIFT;
    return 0;
  }
  assert(false);
}

static PyObject *
long_lshift1(PyLongObject *a, Py_ssize_t wordshift, digit remshift) {
  PyLongObject *z = NULL;
  Py_ssize_t oldsize, newsize, i, j;
  twodigits accum;

  oldsize = Py_ABS(Py_SIZE(a));
  newsize = oldsize + wordshift;
  if (remshift)
    ++newsize;
  z = _PyLong_New(newsize);
  if (z == NULL)
    return NULL;
  if (Py_SIZE(a) < 0) {
    assert(false);
  }
  for (i = 0; i < wordshift; i++)
    z->ob_digit[i] = 0;
  accum = 0;
  for (i = wordshift, j = 0; j < oldsize; i++, j++) {
    accum |= (twodigits) a->ob_digit[j] << remshift;
    z->ob_digit[i] = (digit)(accum & PyLong_MASK);
    accum >>= PyLong_SHIFT;
  }
  if (remshift) {
    z->ob_digit[newsize - 1] = (digit) accum;
  } else
    assert(!accum);
  z = long_normalize(z);
  return (PyObject *) maybe_small_long(z);
}

static PyObject *long_lshift(PyObject *a, PyObject *b) {
  Py_ssize_t wordshift;
  digit remshift;

  CHECK_BINOP(a, b);

  if (Py_SIZE(b) < 0) {
    assert(false);
  }
  if (Py_SIZE(a) == 0) {
    return PyLong_FromLong(0);
  }
  if (divmod_shift(b, &wordshift, &remshift) < 0)
    return NULL;
  return long_lshift1((PyLongObject *) a, wordshift, remshift);
}

static PyObject *
long_rshift1(PyLongObject *a, Py_ssize_t wordshift, digit remshift) {
  PyLongObject *z = NULL;
  Py_ssize_t newsize, hishift, i, j;
  digit lomask, himask;

  if (Py_SIZE(a) < 0) {
    assert(false);
  } else {
    newsize = Py_SIZE(a) - wordshift;
    if (newsize <= 0)
      return PyLong_FromLong(0);
    hishift = PyLong_SHIFT - remshift;
    lomask = ((digit) 1 << hishift) - 1;
    himask = PyLong_MASK ^ lomask;

    z = _PyLong_New(newsize);
    if (z == NULL)
      return NULL;

    for (i = 0, j = wordshift; i < newsize; i++, j++) {
      z->ob_digit[i] = (a->ob_digit[j] >> remshift) & lomask;
      if (i + 1 < newsize)
        z->ob_digit[i] |= (a->ob_digit[j + 1] << hishift) & himask;
    }

    z = maybe_small_long(long_normalize(z));
  }
  return (PyObject *) z;
}

static PyObject *long_rshift(PyObject *a, PyObject *b) {
  Py_ssize_t wordshift;
  digit remshift;

  CHECK_BINOP(a, b);

  if (Py_SIZE(b) < 0) {
    assert(false);
  }
  if (Py_SIZE(a) == 0) {
    return PyLong_FromLong(0);
  }
  if (divmod_shift(b, &wordshift, &remshift) < 0)
    return NULL;
  return long_rshift1((PyLongObject *) a, wordshift, remshift);
}

static PyObject *long_neg(PyLongObject *v) {
  PyLongObject *z;
  if (Py_ABS(Py_SIZE(v)) <= 1)
    return PyLong_FromLong(-MEDIUM_VALUE(v));
  assert(false);
}

static PyObject *long_invert(PyLongObject *v) {
  PyLongObject *x;
  if (Py_ABS(Py_SIZE(v)) <= 1)
    return PyLong_FromLong(-(MEDIUM_VALUE(v) + 1));
  assert(false);
}
