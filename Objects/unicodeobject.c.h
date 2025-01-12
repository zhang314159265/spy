#pragma once

#define STRINGLIB_GET_EMPTY() unicode_get_empty()

#include "internal/pycore_atomic_funcs.h"
#include "methodobject.h"
#include "Objects/stringlib/unicode_format.h"
#include "modsupport.h"

#include "stringlib/asciilib.h"
#include "stringlib/fastsearch.h"
#include "stringlib/partition.h"
#include "stringlib/find_max_char.h"
#include "stringlib/undef.h"

#include "stringlib/ucs1lib.h"
#include "stringlib/fastsearch.h"
#include "stringlib/partition.h"
#include "stringlib/find_max_char.h"
#include "stringlib/undef.h"

#include "stringlib/unicodedefs.h"
#include "stringlib/find.h"

#define MAX_UNICODE 0x10ffff

#define LEFTSTRIP 0
#define RIGHTSTRIP 1

#undef STRINGLIB_GET_EMPTY

#define ADJUST_INDICES(start, end, len) \
  if (end > len) \
    end = len; \
  else if (end < 0) { \
    end += len; \
    if (end < 0) \
      end = 0; \
  } \
  if (start < 0) { \
    start += len; \
    if (start < 0) \
      start = 0; \
  }


PyObject * _PyUnicode_FromId(_Py_Identifier *id)
{
  PyInterpreterState *interp = _PyInterpreterState_GET();
  struct _Py_unicode_ids *ids = &interp->unicode.ids;

	Py_ssize_t index = _Py_atomic_size_get(&id->index);
	if (index < 0) {
		struct _Py_unicode_runtime_ids *rt_ids = &interp->runtime->unicode_ids;
		PyThread_acquire_lock(rt_ids->lock, WAIT_LOCK);
		index = _Py_atomic_size_get(&id->index);
		if (index < 0) {
			index = rt_ids->next_index;
			rt_ids->next_index++;
			_Py_atomic_size_set(&id->index, index);
		}
		PyThread_release_lock(rt_ids->lock);
	}
	assert(index >= 0);

	PyObject *obj;
	if (index < ids->size) {
		obj = ids->array[index];
		if (obj) {
			return obj;
		}
	}

	obj = PyUnicode_DecodeUTF8Stateful(id->string, strlen(id->string),
			NULL, NULL);
	if (!obj) {
		return NULL;
	}
	PyUnicode_InternInPlace(&obj);

	if (index >= ids->size) {
		Py_ssize_t new_size = Py_MAX(index * 2, 16);
		Py_ssize_t item_size = sizeof(ids->array[0]);
		PyObject **new_array = PyMem_Realloc(ids->array, new_size * item_size);
		if (new_array == NULL) {
			assert(false);
		}
		memset(&new_array[ids->size], 0, (new_size - ids->size) * item_size);
		ids->array = new_array;
		ids->size = new_size;
	}

	ids->array[index] = obj;

	return obj;
}

PyDoc_STRVAR(format__doc__, "");

static int
ensure_unicode(PyObject *obj) {
  if (!PyUnicode_Check(obj)) {
    fail(0);
  }
  return PyUnicode_READY(obj);
}

PyObject *
PyUnicode_RPartition(PyObject *str_obj, PyObject *sep_obj) {
  PyObject *out;
  int kind1, kind2;
  const void *buf1, *buf2;
  Py_ssize_t len1, len2;

  if (ensure_unicode(str_obj) < 0 || ensure_unicode(sep_obj) < 0)
    return NULL;

  kind1 = PyUnicode_KIND(str_obj);
  kind2 = PyUnicode_KIND(sep_obj);
  len1 = PyUnicode_GET_LENGTH(str_obj);
  len2 = PyUnicode_GET_LENGTH(sep_obj);
  if (kind1 < kind2 || len1 < len2) {
    PyObject *empty = unicode_get_empty();
    return PyTuple_Pack(3, empty, empty, str_obj);
  }
  buf1 = PyUnicode_DATA(str_obj);
  buf2 = PyUnicode_DATA(sep_obj);
  if (kind2 != kind1) {
    fail(0);
  }
  switch (kind1) {
  case PyUnicode_1BYTE_KIND:
    if (PyUnicode_IS_ASCII(str_obj) && PyUnicode_IS_ASCII(sep_obj))
      out = asciilib_rpartition(str_obj, buf1, len1, sep_obj, buf2, len2);
    else {
      fail(0);
    }
    break;
  default:
    fail(0);
  }
  assert((kind2 == kind1) == (buf2 == PyUnicode_DATA(sep_obj)));
  if (kind2 != kind1)
    PyMem_Free((void *) buf2);
  return out;
}

static PyObject *
unicode_rpartition(PyObject *self, PyObject *sep) {
  return PyUnicode_RPartition(self, sep);
}

#define UNICODE_RPARTITION_METHODDEF \
  {"rpartition", (PyCFunction)unicode_rpartition, METH_O, ""},

PyObject *
_PyUnicode_JoinArray(PyObject *separator, PyObject *const *items, Py_ssize_t seqlen) {
  PyObject *res = NULL;
  PyObject *sep = NULL;
  Py_ssize_t seplen;
  PyObject *item;
  Py_ssize_t sz, i, res_offset;
  Py_UCS4 maxchar;
  Py_UCS4 item_maxchar;
  int use_memcpy;
  unsigned char *res_data = NULL, *sep_data = NULL;
  PyObject *last_obj;
  unsigned int kind = 0;

  if (seqlen == 0) {
    fail(0);
  }

  last_obj = NULL;
  if (seqlen == 1) {
    if (PyUnicode_CheckExact(items[0])) {
      res = items[0];
      Py_INCREF(res);
      return res;
    }
    fail(0);
  } else {
    if (separator == NULL) {
      fail(0);
    } else {
      if (!PyUnicode_Check(separator)) {
        fail(0);
      }
      if (PyUnicode_READY(separator))
        fail(0);
      sep = separator;
      seplen = PyUnicode_GET_LENGTH(separator);
      maxchar = PyUnicode_MAX_CHAR_VALUE(separator);
      Py_INCREF(sep);
    }
    last_obj = sep;
  }

  sz = 0;
  use_memcpy = 1;
  for (i = 0; i < seqlen; i++) {
    size_t add_sz;
    item = items[i];
    if (!PyUnicode_Check(item)) {
      fail(0);
    }
    if (PyUnicode_READY(item) == -1)
      fail(0);
    add_sz = PyUnicode_GET_LENGTH(item);
    item_maxchar = PyUnicode_MAX_CHAR_VALUE(item);
    maxchar = Py_MAX(maxchar, item_maxchar);
    if (i != 0) {
      add_sz += seplen;
    }
    if (add_sz > (size_t)(PY_SSIZE_T_MAX - sz)) {
      fail(0);
    }
    sz += add_sz;
    if (use_memcpy && last_obj != NULL) {
      if (PyUnicode_KIND(last_obj) != PyUnicode_KIND(item))
        use_memcpy = 0;
    }
    last_obj = item;
  }

  res = PyUnicode_New(sz, maxchar);
  if (res == NULL)
    fail(0);

  if (use_memcpy) {
    res_data = PyUnicode_1BYTE_DATA(res);
    kind = PyUnicode_KIND(res);
    if (seplen != 0)
      sep_data = PyUnicode_1BYTE_DATA(sep);
  }
  if (use_memcpy) {
    for (i = 0; i < seqlen; ++i) {
      Py_ssize_t itemlen;
      item = items[i];

      if (i && seplen != 0) {
        memcpy(res_data, sep_data, kind * seplen);
        res_data += kind * seplen;
      }
      itemlen = PyUnicode_GET_LENGTH(item);
      if (itemlen != 0) {
        memcpy(res_data, PyUnicode_DATA(item), kind * itemlen);
        res_data += kind * itemlen;
      }
    }
    assert(res_data == PyUnicode_1BYTE_DATA(res)
      + kind * PyUnicode_GET_LENGTH(res));
  } else {
    fail(0);
  }
  Py_XDECREF(sep);
  return res;
}

PyObject *
PyUnicode_Join(PyObject *separator, PyObject *seq) {
  PyObject *res;
  PyObject *fseq;
  Py_ssize_t seqlen;
  PyObject **items;

  fseq = PySequence_Fast(seq, "can only join an iterable");
  if (fseq == NULL) {
    return NULL;
  }
  items = PySequence_Fast_ITEMS(fseq);
  seqlen = PySequence_Fast_GET_SIZE(fseq);
  res = _PyUnicode_JoinArray(separator, items, seqlen);
  Py_DECREF(fseq);
  return res;
}

static PyObject *
unicode_join(PyObject *self, PyObject *iterable) {
  return PyUnicode_Join(self, iterable);
}

#define UNICODE_JOIN_METHODDEF \
  {"join", (PyCFunction) unicode_join, METH_O, ""},

static int
tailmatch(PyObject *self,
    PyObject *substring,
    Py_ssize_t start,
    Py_ssize_t end,
    int direction) {

  int kind_self;
  int kind_sub;
  const void *data_self;
  const void *data_sub;
  Py_ssize_t offset;
  Py_ssize_t i;
  Py_ssize_t end_sub;

  if (PyUnicode_READY(self) == -1 ||
      PyUnicode_READY(substring) == -1)
    return -1;

  ADJUST_INDICES(start, end, PyUnicode_GET_LENGTH(self));
  end -= PyUnicode_GET_LENGTH(substring);
  if (end < start) {
    return 0;
  }
  if (PyUnicode_GET_LENGTH(substring) == 0)
    return 1;

  kind_self = PyUnicode_KIND(self);
  data_self = PyUnicode_DATA(self);
  kind_sub = PyUnicode_KIND(substring);
  data_sub = PyUnicode_DATA(substring);
  end_sub = PyUnicode_GET_LENGTH(substring) - 1;
  
  if (direction > 0)
    offset = end;
  else
    offset = start;

  if (PyUnicode_READ(kind_self, data_self, offset) ==
    PyUnicode_READ(kind_sub, data_sub, 0) &&
    PyUnicode_READ(kind_self, data_self, offset + end_sub) ==
    PyUnicode_READ(kind_sub, data_sub, end_sub)) {

    if (kind_self == kind_sub) {
      return !memcmp((char *) data_self + (offset * PyUnicode_KIND(substring)),
          data_sub,
          PyUnicode_GET_LENGTH(substring) * PyUnicode_KIND(substring));
    } else {
      fail(0);
    }
  }

  return 0;
}

static PyObject *
unicode_endswith(PyObject *self, PyObject *args) {
  PyObject *subobj;
  PyObject *substring;
  Py_ssize_t start = 0;
  Py_ssize_t end = PY_SSIZE_T_MAX;
  int result;

  if (!stringlib_parse_args_finds("endswith", args, &subobj, &start, &end))
    return NULL;
  if (PyTuple_Check(subobj)) {
    Py_ssize_t i;
    for (i = 0; i < PyTuple_GET_SIZE(subobj); i++) {
      substring = PyTuple_GET_ITEM(subobj, i);
      if (!PyUnicode_Check(substring)) {
        fail(0);
      }
      result = tailmatch(self, substring, start, end, +1);
      if (result == -1)
        return NULL;
      if (result) {
        Py_RETURN_TRUE;
      }
    }
    Py_RETURN_FALSE;
  }
  if (!PyUnicode_Check(subobj)) {
    fail(0);
  }
  result = tailmatch(self, subobj, start, end, +1);
  if (result == -1)
    return NULL;
  return PyBool_FromLong(result);
}



static PyObject *
unicode_startswith(PyObject *self, PyObject *args) {
  PyObject *subobj;
  PyObject *substring;
  Py_ssize_t start = 0;
  Py_ssize_t end = PY_SSIZE_T_MAX;
  int result;

  if (!stringlib_parse_args_finds("startswith", args, &subobj, &start, &end))
    return NULL;
  if (PyTuple_Check(subobj)) {
    Py_ssize_t i;
    for (i = 0; i < PyTuple_GET_SIZE(subobj); i++) {
      substring = PyTuple_GET_ITEM(subobj, i);
      if (!PyUnicode_Check(substring)) {
        fail(0);
      }
      result = tailmatch(self, substring, start, end, -1);
      if (result == -1)
        return NULL;
      if (result) {
        Py_RETURN_TRUE;
      }
    }
    Py_RETURN_FALSE;
  }
  if (!PyUnicode_Check(subobj)) {
    fail(0);
  }
  result = tailmatch(self, subobj, start, end, -1);
  if (result == -1)
    return NULL;
  return PyBool_FromLong(result);
}

PyObject *
_PyUnicode_XStrip(PyObject *self, int striptype, PyObject *sepobj) {
  const void *data;
  int kind;
  Py_ssize_t i, j, len;
  Py_ssize_t seplen;

  if (PyUnicode_READY(self) == -1 || PyUnicode_READY(sepobj) == -1)
    return NULL;

  kind = PyUnicode_KIND(self);
  data = PyUnicode_DATA(self);
  len = PyUnicode_GET_LENGTH(self);
  seplen = PyUnicode_GET_LENGTH(sepobj);

  i = 0;
  if (striptype != RIGHTSTRIP) {
    while (i < len) {
      Py_UCS4 ch = PyUnicode_READ(kind, data, i);
      if (PyUnicode_FindChar(sepobj, ch, 0, seplen, 1) < 0)
        break;
      i++;
    }
  }

  j = len;
  if (striptype != LEFTSTRIP) {
    j--;
    while (j >= i) {
      Py_UCS4 ch = PyUnicode_READ(kind, data, j);
      if (PyUnicode_FindChar(sepobj, ch, 0, seplen, 1) < 0)
        break;
      j--;
    }
    j++;
  }

  return PyUnicode_Substring(self, i, j);
}

static PyObject *
do_argstrip(PyObject *self, int striptype, PyObject *sep) {
  if (sep != Py_None) {
    if (PyUnicode_Check(sep)) {
      return _PyUnicode_XStrip(self, striptype, sep);
    } else {
      fail(0);
    }
  }
  fail(0);
}

static PyObject *
unicode_rstrip_impl(PyObject *self, PyObject *chars) {
  return do_argstrip(self, RIGHTSTRIP, chars);
}

static PyObject *
unicode_rstrip(PyObject *self, PyObject *const *args, Py_ssize_t nargs) {
  PyObject *return_value = NULL;
  PyObject *chars = Py_None;

  if (!_PyArg_CheckPositional("rstrip", nargs, 0, 1)) {
    goto exit;
  }

  if (nargs < 1) {
    goto skip_optional;
  }
  chars = args[0];
skip_optional:
  return_value = unicode_rstrip_impl(self, chars);

exit:
  return return_value;
}


#define UNICODE_RSTRIP_METHODDEF \
  {"rstrip", (PyCFunction)(void(*)(void)) unicode_rstrip, METH_FASTCALL, ""},

#define UNICODE_LOWER_METHODDEF \
  {"lower", (PyCFunction) unicode_lower, METH_NOARGS, ""},

void
_Py_bytes_lower(char *result, const char *cptr, Py_ssize_t len) {
  Py_ssize_t i;

  for (i = 0; i < len; i++) {
    // TODO follow cpy
    // result[i] = Py_TOLOWER((unsigned char) cptr[i]);
    result[i] = tolower((unsigned char) cptr[i]);
  }
}

static PyObject *
ascii_upper_or_lower(PyObject *self, int lower) {
  Py_ssize_t len = PyUnicode_GET_LENGTH(self);
  const char *data = PyUnicode_DATA(self);
  char *resdata;
  PyObject *res;
  res = PyUnicode_New(len, 127);
  if (res == NULL)
    return NULL;
  resdata = PyUnicode_DATA(res);
  if (lower)
    _Py_bytes_lower(resdata, data, len);
  else
    fail(0);
  return res;
}

static PyObject *
unicode_lower_impl(PyObject *self) {
  if (PyUnicode_READY(self) == -1)
    return NULL;

  if (PyUnicode_IS_ASCII(self))
    return ascii_upper_or_lower(self, 1);
  fail(0);
}

static PyObject *
unicode_lower(PyObject *self, PyObject *ignored) {
  return unicode_lower_impl(self);
}

static inline int
parse_args_finds_unicode(const char *function_name, PyObject *args,
    PyObject **substring,
    Py_ssize_t *start, Py_ssize_t *end) {
  if (stringlib_parse_args_finds(function_name, args, substring,
      start, end)) {
    if (ensure_unicode(*substring) < 0)
      return 0;
    return 1;
  }
  return 0;
}

#define ADJUST_INDICES(start, end, len) \
  if (end > len) \
    end = len; \
  else if (end < 0) { \
    end += len; \
    if (end < 0) \
      end = 0; \
  } \
  if (start < 0) { \
    start += len; \
    if (start < 0) \
      start = 0; \
  }

static inline Py_ssize_t findchar(const void *s, int kind, Py_ssize_t size, Py_UCS4 ch, int direction);

static Py_ssize_t
any_find_slice(PyObject *s1, PyObject *s2,
    Py_ssize_t start,
    Py_ssize_t end,
    int direction) {
  int kind1, kind2;
  const void *buf1, *buf2;
  Py_ssize_t len1, len2, result;

  kind1 = PyUnicode_KIND(s1);
  kind2 = PyUnicode_KIND(s2);
  if (kind1 < kind2)
    return -1;

  len1 = PyUnicode_GET_LENGTH(s1);
  len2 = PyUnicode_GET_LENGTH(s2);
  ADJUST_INDICES(start, end, len1);
  if (end - start < len2)
    return -1;

  buf1 = PyUnicode_DATA(s1);
  buf2 = PyUnicode_DATA(s2);
  if (len2 == 1) {
    Py_UCS4 ch = PyUnicode_READ(kind2, buf2, 0);
    result = findchar((const char *) buf1 + kind1 * start,
      kind1, end - start, ch, direction);
    if (result == -1)
      return -1;
    else
      return start + result;
  }
  fail(0);
}

static PyObject *
unicode_rfind(PyObject *self, PyObject *args) {
  PyObject *substring = NULL;
  Py_ssize_t start = 0;
  Py_ssize_t end = 0;
  Py_ssize_t result;

  if (!parse_args_finds_unicode("rfind", args, &substring, &start, &end))
    return NULL;

  if (PyUnicode_READY(self) == -1)
    return NULL;

  result = any_find_slice(self, substring, start, end, -1);

  if (result == -2)
    return NULL;

  return PyLong_FromSsize_t(result);
}

static PyMethodDef unicode_methods[] = {
	{"format", (PyCFunction)(void(*)(void)) do_string_format, METH_VARARGS | METH_KEYWORDS, format__doc__},

  UNICODE_RPARTITION_METHODDEF
  UNICODE_JOIN_METHODDEF
  UNICODE_RSTRIP_METHODDEF
  UNICODE_LOWER_METHODDEF
  {"startswith", (PyCFunction) unicode_startswith, METH_VARARGS, ""},
  {"endswith", (PyCFunction) unicode_endswith, METH_VARARGS, ""},
  {"rfind", (PyCFunction) unicode_rfind, METH_VARARGS, ""},
  {NULL, NULL},
};

static PyObject *
unicode_repr(PyObject *unicode) {
  PyObject *repr;
  Py_ssize_t isize;
  Py_ssize_t osize, squote, dquote, i, o;
  Py_UCS4 max, quote;
  int ikind, okind, unchanged;
  const void *idata;
  void *odata;

  if (PyUnicode_READY(unicode) == -1)
    return NULL;

  isize = PyUnicode_GET_LENGTH(unicode);
  idata = PyUnicode_DATA(unicode);

  osize = 0;
  max = 127;
  squote = dquote = 0;
  ikind = PyUnicode_KIND(unicode);
  for (i = 0; i < isize; i++) {
    Py_UCS4 ch = PyUnicode_READ(ikind, idata, i);
    Py_ssize_t incr = 1;
    switch (ch) {
    case '\'': squote++; break;
    case '"': dquote++; break;
    case '\\': case '\t': case '\r': case '\n':
      incr = 2;
      break;
    default:
      if (ch < ' ' || ch == 0x7f)
        incr = 4;
      else if (ch < 0x7f)
        ;
      else {
        fail(0);
      }
    }
    osize += incr;
  }

  quote = '\'';
  unchanged = (osize == isize);
  if (squote) {
    fail(0);
  }

  osize += 2;

  repr = PyUnicode_New(osize, max);
  if (repr == NULL)
    return NULL;
  okind = PyUnicode_KIND(repr);
  odata = PyUnicode_DATA(repr);

  PyUnicode_WRITE(okind, odata, 0, quote);
  PyUnicode_WRITE(okind, odata, osize - 1, quote);

  if (unchanged) {
    _PyUnicode_FastCopyCharacters(repr, 1,
      unicode, 0,
      isize);
  } else {
    fail(0);
  }

  return repr;
}

static PyObject *unicode_new(PyTypeObject *type, PyObject *args, PyObject *kwargs);

static Py_ssize_t
unicode_length(PyObject *self) {
  if (PyUnicode_READY(self) == -1)
    return -1;
  return PyUnicode_GET_LENGTH(self);
}

PyObject *
PyUnicode_Concat(PyObject *left, PyObject *right) {
  fail(0);
}

static PyObject *
unicode_getitem(PyObject *self, Py_ssize_t index) {
  fail(0);
}

static PySequenceMethods unicode_as_sequence = {
  .sq_length = (lenfunc) unicode_length,
  .sq_concat = PyUnicode_Concat,
  .sq_item = (ssizeargfunc) unicode_getitem,
};

static PyObject *
unicode_subscript(PyObject *self, PyObject *item) {
  if (PyUnicode_READY(self) == -1)
    return NULL;

  if (_PyIndex_Check(item)) {
    fail(0);
  } else if (PySlice_Check(item)) {
    Py_ssize_t start, stop, step, slicelength;

    if (PySlice_Unpack(item, &start, &stop, &step) < 0) {
      return NULL;
    }
    slicelength = PySlice_AdjustIndices(PyUnicode_GET_LENGTH(self),
        &start, &stop, step);
    if (slicelength <= 0) {
      fail(0);
    } else if (start == 0 && step == 1 &&
        slicelength == PyUnicode_GET_LENGTH(self)) {
      fail(0);
    } else if (step == 1) {
      return PyUnicode_Substring(self,
        start, start + slicelength);
    }

    // General case
    fail(0);
  } else {
    fail(0);
  }
}

static PyMappingMethods unicode_as_mapping = {
  .mp_subscript = (binaryfunc) unicode_subscript,
};

typedef struct {
  PyObject_HEAD
  Py_ssize_t it_index;
  PyObject *it_seq;
} unicodeiterobject;

static void
unicodeiter_dealloc(unicodeiterobject *it) {
  Py_XDECREF(it->it_seq);
  PyObject_GC_Del(it);
}

static PyObject *
get_latin1_char(Py_UCS1 ch) {
  // TODO follow cpy
  PyObject *unicode = PyUnicode_New(1, ch);
  if (!unicode) {
    return NULL;
  }

  PyUnicode_1BYTE_DATA(unicode)[0] = ch;
  Py_INCREF(unicode);
  return unicode;
}

static PyObject *
unicode_char(Py_UCS4 ch) {
  assert(ch <= MAX_UNICODE);

  if (ch < 256) {
    return get_latin1_char(ch);
  }
  fail(0);
}

PyObject *
PyUnicode_FromOrdinal(int ordinal) {
  if (ordinal < 0 || ordinal > MAX_UNICODE) {
    fail(0);
  }
  return unicode_char((Py_UCS4) ordinal);
}

static PyObject *
unicodeiter_next(unicodeiterobject *it) {
  PyObject *seq, *item;

  assert(it != NULL);
  seq = it->it_seq;
  if (seq == NULL)
    return NULL;
  assert(_PyUnicode_CHECK(seq));

  if (it->it_index < PyUnicode_GET_LENGTH(seq)) {
    int kind = PyUnicode_KIND(seq);
    const void *data = PyUnicode_DATA(seq);
    Py_UCS4 chr = PyUnicode_READ(kind, data, it->it_index);
    item = PyUnicode_FromOrdinal(chr);
    if (item != NULL)
      ++it->it_index;
    return item;
  }
  it->it_seq = NULL;
  Py_DECREF(seq);
  return NULL;
}

static PyMethodDef unicodeiter_methods[] = {
  {NULL, NULL},
};

PyTypeObject PyUnicodeIter_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "str_iterator",
  .tp_basicsize = sizeof(unicodeiterobject),
  .tp_dealloc = (destructor) unicodeiter_dealloc,
  .tp_getattro = PyObject_GenericGetAttr,
  .tp_iter = PyObject_SelfIter,
  .tp_iternext = (iternextfunc) unicodeiter_next,
  .tp_methods = unicodeiter_methods,
};

static PyObject *
unicode_iter(PyObject *seq) {
  unicodeiterobject *it;
  
  if (!PyUnicode_Check(seq)) {
    fail(0);
  }
  if (PyUnicode_READY(seq) == -1) {
    return NULL;
  }

  it = PyObject_GC_New(unicodeiterobject, &PyUnicodeIter_Type);
  if (it == NULL)
    return NULL;
  it->it_index = 0;
  Py_INCREF(seq);
  it->it_seq = seq;
  return (PyObject *) it;
}

// defined in cpy/Objects/unicodeobject.c
PyTypeObject PyUnicode_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "str",
  .tp_basicsize = sizeof(PyUnicodeObject),
	.tp_flags = Py_TPFLAGS_UNICODE_SUBCLASS,
	.tp_hash = (hashfunc) unicode_hash,
	.tp_dealloc = (destructor) unicode_dealloc,
  .tp_richcompare = PyUnicode_RichCompare,
  .tp_getattro = PyObject_GenericGetAttr,
  .tp_methods = unicode_methods,
  .tp_repr = unicode_repr,
  .tp_new = unicode_new,
  .tp_as_sequence = &unicode_as_sequence,
  .tp_as_mapping = &unicode_as_mapping,
  .tp_iter = unicode_iter,
};

Py_UCS4
_PyUnicode_FindMaxChar(PyObject *unicode, Py_ssize_t start, Py_ssize_t end) {
	// TODO follow cpy
	return 127;
}

static int
_copy_characters(PyObject *to, Py_ssize_t to_start,
		PyObject *from, Py_ssize_t from_start,
		Py_ssize_t how_many, int check_maxchar) {
	unsigned int from_kind, to_kind;
	const void *from_data;
	void *to_data;

	if (how_many == 0)
		return 0;

	from_kind = PyUnicode_KIND(from);
	from_data = PyUnicode_DATA(from);
	to_kind = PyUnicode_KIND(to);
	to_data = PyUnicode_DATA(to);

	if (from_kind == to_kind) {
		if (check_maxchar) {
			assert(false);
		}
		memcpy((char *) to_data + to_kind * to_start,
			(const char *) from_data + from_kind * from_start,
			to_kind * how_many);
	} else {
		assert(false);
	}

	return 0;
}

void 
_PyUnicode_FastCopyCharacters(
		PyObject *to, Py_ssize_t to_start,
		PyObject *from, Py_ssize_t from_start, Py_ssize_t how_many) {
	(void)_copy_characters(to, to_start, from, from_start, how_many, 0);
}

int
_PyUnicodeWriter_WriteSubstring(_PyUnicodeWriter *writer, PyObject *str,
		Py_ssize_t start, Py_ssize_t end) {
	Py_UCS4 maxchar;
	Py_ssize_t len;

	if (PyUnicode_READY(str) == -1)
		return -1;
	
	assert(0 <= start);
	assert(end <= PyUnicode_GET_LENGTH(str));
	assert(start <= end);

	if (end == 0)
		return 0;
	
	if (start == 0 && end == PyUnicode_GET_LENGTH(str))
		assert(false);
	
	if (PyUnicode_MAX_CHAR_VALUE(str) > writer->maxchar)
		maxchar = _PyUnicode_FindMaxChar(str, start, end);
	else
		maxchar = writer->maxchar;
	
	len = end - start;

	if (_PyUnicodeWriter_Prepare(writer, len, maxchar) < 0)
		return -1;
	_PyUnicode_FastCopyCharacters(writer->buffer, writer->pos,
			str, start, len);

	writer->pos += len;
	return 0;
}

int
_PyUnicode_EqualToASCIIId(PyObject *left, _Py_Identifier *right) {
  PyObject *right_uni;

  assert(_PyUnicode_CHECK(left));
  assert(right->string);

  if (PyUnicode_READY(left) == -1) {
    fail(0);
  }

  if (!PyUnicode_IS_ASCII(left))
    return 0;

  right_uni = _PyUnicode_FromId(right);
  if (right_uni == NULL) {
    fail(0);
  }

  if (left == right_uni)
    return 1;

  if (PyUnicode_CHECK_INTERNED(left)) {
    return 0;
  }

  return unicode_compare_eq(left, right_uni);
}

static int
unicode_modifiable(PyObject *unicode) {
  assert(_PyUnicode_CHECK(unicode));
  if (Py_REFCNT(unicode) != 1)
    return 0;
  assert(false);
}

void PyUnicode_Append(PyObject **p_left, PyObject *right) {
  PyObject *left, *res;
  Py_UCS4 maxchar, maxchar2;
  Py_ssize_t left_len, right_len, new_len;

  if (p_left == NULL) {
    assert(false);
  }
  left = *p_left;
  if (right == NULL || left == NULL
      || !PyUnicode_Check(left) || !PyUnicode_Check(right)) {
    assert(false);
  }

  if (PyUnicode_READY(left) == -1)
    goto error;
  if (PyUnicode_READY(right) == -1)
    goto error;

  PyObject *empty = unicode_get_empty();
  if (left == empty) {
    assert(false);
  }
  if (right == empty) {
    return;
  }

  left_len = PyUnicode_GET_LENGTH(left);
  right_len = PyUnicode_GET_LENGTH(right);
  if (left_len > PY_SSIZE_T_MAX - right_len) {
    assert(false);
  }

  new_len = left_len + right_len;
  if (unicode_modifiable(left)
      && PyUnicode_CheckExact(right)
      && PyUnicode_KIND(right) <= PyUnicode_KIND(left)
      && !(PyUnicode_IS_ASCII(left) && !PyUnicode_IS_ASCII(right)))
  {
    assert(false);
  } else {
    maxchar = PyUnicode_MAX_CHAR_VALUE(left);
    maxchar2 = PyUnicode_MAX_CHAR_VALUE(right);
    maxchar = Py_MAX(maxchar, maxchar2);

    res = PyUnicode_New(new_len, maxchar);
    if (res == NULL)
      goto error;
    _PyUnicode_FastCopyCharacters(res, 0, left, 0, left_len);
    _PyUnicode_FastCopyCharacters(res, left_len, right, 0, right_len);
    Py_DECREF(left);
    *p_left = res;
  }
  return;
error:
  Py_CLEAR(*p_left);
}

static inline Py_ssize_t
findchar(const void *s, int kind,
    Py_ssize_t size, Py_UCS4 ch,
    int direction) {
  switch (kind) {
  case PyUnicode_1BYTE_KIND:
    if ((Py_UCS1) ch != ch)
      return -1;
    if (direction > 0)
      return ucs1lib_find_char((const Py_UCS1 *) s, size, (Py_UCS1) ch);
    else
      return ucs1lib_rfind_char((const Py_UCS1 *) s, size, (Py_UCS1) ch);
  default:
    assert(false);
  }
}

Py_ssize_t PyUnicode_FindChar(PyObject *str, Py_UCS4 ch,
    Py_ssize_t start, Py_ssize_t end,
    int direction) {
  int kind;
  Py_ssize_t len, result;

  if (PyUnicode_READY(str) == -1)
    return -2;
  len = PyUnicode_GET_LENGTH(str);
  ADJUST_INDICES(start, end, len);
  if (end - start < 1)
    return -1;
  kind = PyUnicode_KIND(str);
  result = findchar(PyUnicode_1BYTE_DATA(str) + kind * start,
    kind, end - start, ch, direction);
  if (result == -1)
    return -1;
  else
    return start + result;
}

#define _Py_RETURN_UNICODE_EMPTY() \
  do { \
    return unicode_new_empty(); \
  } while (0)

static PyObject *_PyUnicode_FromUCS1(const Py_UCS1* u, Py_ssize_t size) {
  PyObject *res;
  unsigned char max_char;

  if (size == 0) {
    _Py_RETURN_UNICODE_EMPTY();
  }
  assert(size > 0);
  #if 0
  if (size == 1) {
    assert(false);
  }
  #endif

  // printf("_PyUnicode_FromUCS1 for '%.*s'\n", (int) size, (char *) u);

  max_char = ucs1lib_find_max_char(u, u + size);
  res = PyUnicode_New(size, max_char);
  if (!res)
    return NULL;
  memcpy(PyUnicode_1BYTE_DATA(res), u, size);
  return res;
}

static PyObject *
unicode_new_impl(PyTypeObject *type, PyObject *x, const char *encoding, const char *errors) {
  PyObject *unicode;
  if (x == NULL) {
    fail(0);
  } else if (encoding == NULL && errors == NULL) {
    unicode = PyObject_Str(x);
  } else {
    fail(0);
  }

  if (unicode != NULL && type != &PyUnicode_Type) {
    fail(0);
  }
  return unicode;
}

static PyObject *unicode_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
  PyObject *return_value = NULL;
  static const char * const _keywords[] = {"object", "encoding", "errors", NULL};
  static _PyArg_Parser _parser = {NULL, _keywords, "str", 0};
  PyObject *argsbuf[3];
  PyObject * const *fastargs;
  Py_ssize_t nargs = PyTuple_GET_SIZE(args);
  Py_ssize_t noptargs = nargs + (kwargs ? PyDict_GET_SIZE(kwargs) : 0) - 0;
  PyObject *x = NULL;
  const char *encoding = NULL;
  const char *errors = NULL;

  fastargs = _PyArg_UnpackKeywords(_PyTuple_CAST(args)->ob_item, nargs, kwargs, NULL, &_parser, 0, 3, 0, argsbuf);
  if (!fastargs) {
    fail(0);
  }
  if (!noptargs) {
    fail(0);
  }
  if (fastargs[0]) {
    x = fastargs[0];
    if (!--noptargs) {
      goto skip_optional_pos;
    }
  }
  if (fastargs[1]) {
    fail(0);
  }
  if (!PyUnicode_Check(fastargs[2])) {
    fail(0);
  }
  fail(0);

skip_optional_pos:
  return_value = unicode_new_impl(type, x, encoding, errors);
exit:
  return return_value;
}

PyObject *
PyUnicode_DecodeFSDefaultAndSize(const char *s, Py_ssize_t size) {
  // TODO follow cpy
  return PyUnicode_FromStringAndSize(s, size);
}


PyObject *PyUnicode_DecodeFSDefault(const char *s) {
  Py_ssize_t size = (Py_ssize_t) strlen(s);
  return PyUnicode_DecodeFSDefaultAndSize(s, size);
}

PyObject * unicode_decode_utf8(const char *s, Py_ssize_t size, _Py_error_handler error_handler, const char *errors, Py_ssize_t *consumed) {
	if (size == 0) {
		if (consumed) {
			*consumed = 0;
		}
		_Py_RETURN_UNICODE_EMPTY();
	}
  #if 0
	// ASCII is equivalent to the first 128 ordinals in Unicode
	if (size == 1 && (unsigned char) s[0] < 128) {
		assert(false);
	}
  #endif

	const char *starts = s;
	const char *end = s + size;

	// fast path: try ASCII string.
	PyObject *u = PyUnicode_New(size, 127);
	if (u == NULL) {
		return NULL;
	}
	s += ascii_decode(s, end, PyUnicode_1BYTE_DATA(u));
	if (s == end) {
		return u;
	}
	assert(false);
}


static PyObject *
unicode_encode_utf8(PyObject *unicode, _Py_error_handler error_handler, const char *errors) {
  if (!PyUnicode_Check(unicode)) {
    fail(0);
  }

  if (PyUnicode_READY(unicode) == -1)
    return NULL;
  
  if (PyUnicode_UTF8(unicode)) {
    return PyBytes_FromStringAndSize(PyUnicode_UTF8(unicode),
        PyUnicode_UTF8_LENGTH(unicode));
  }

  fail(0);
}

PyObject *
PyUnicode_EncodeFSDefault(PyObject *unicode) {
  // TODO follow cpy
  _Py_error_handler errors = _Py_ERROR_UNKNOWN;
  return unicode_encode_utf8(unicode, errors, NULL);
}

int PyUnicode_FSConverter(PyObject *arg, void *addr) {
  PyObject *path = NULL;
  PyObject *output = NULL;
  Py_ssize_t size;
  const char *data;

  if (arg == NULL) {
    fail(0);
  }
  path = PyOS_FSPath(arg);
  if (path == NULL) {
    return 0;
  }
  if (PyBytes_Check(path)) {
    output = path;
  } else {
    output = PyUnicode_EncodeFSDefault(path);
    Py_DECREF(path);
    if (!output) {
      return 0;
    }
    assert(PyBytes_Check(output));
  }

  size = PyBytes_GET_SIZE(output);
  data = PyBytes_AS_STRING(output);
  if ((size_t) size != strlen(data)) {
    fail(0);
  }
  *(PyObject **) addr = output;
  return Py_CLEANUP_SUPPORTED;
}


int PyUnicode_FSDecoder(PyObject *arg, void *addr) {
  int is_buffer = 0;
  PyObject *path = NULL;
  PyObject *output = NULL;
  if (arg == NULL) {
    fail(0);
  }

  is_buffer = PyObject_CheckBuffer(arg);
  if (!is_buffer) {
    path = PyOS_FSPath(arg);
    if (path == NULL) {
      return 0;
    }
  } else {
    path = arg;
    Py_INCREF(arg);
  }

  if (PyUnicode_Check(path)) {
    output = path;
  } else if (PyBytes_Check(path) || is_buffer) {
    fail(0);
  } else {
    fail(0);
  }
  if (PyUnicode_READY(output) == -1) {
    fail(0);
  }
  if (findchar(PyUnicode_DATA(output), PyUnicode_KIND(output),
      PyUnicode_GET_LENGTH(output), 0, 1) >= 0) {
    fail(0);
  }
  *(PyObject **) addr = output;
  return Py_CLEANUP_SUPPORTED;
  fail(0);
}
