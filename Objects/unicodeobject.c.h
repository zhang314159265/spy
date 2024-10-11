#pragma once

#include "internal/pycore_atomic_funcs.h"

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
