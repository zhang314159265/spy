#pragma once

#include "internal/pycore_atomic_funcs.h"
#include "methodobject.h"
#include "Objects/stringlib/unicode_format.h"

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

static PyMethodDef unicode_methods[] = {
	{"format", (PyCFunction)(void(*)(void)) do_string_format, METH_VARARGS | METH_KEYWORDS, format__doc__},
  {NULL, NULL},
};

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
  assert(false);
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
