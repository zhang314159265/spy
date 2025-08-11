#pragma once

#include <ffi.h>

// how to decode the size field for interger get/set functions
#define LOW_BIT(x) ((x) & 0xFFFF)
#define NUM_BITS(x) ((x) >> 16)

#define GET_BITFIELD(v, size) \
  if (NUM_BITS(size)) { \
    fail("GET_BITFIELD NYI"); \
  }

static PyObject *
i_set(void *ptr, PyObject *value, Py_ssize_t size) {
  fail(0);
}

static PyObject *
i_set_sw(void *ptr, PyObject *value, Py_ssize_t size) {
  fail(0);
}

static PyObject *
i_get(void *ptr, Py_ssize_t size) {
  int val;
  memcpy(&val, ptr, sizeof(val));
  GET_BITFIELD(val, size);
  return PyLong_FromLong(val);
}

static PyObject *
i_get_sw(void *ptr, Py_ssize_t size) {
  fail(0);
}

static PyObject *
P_set(void *ptr, PyObject *value, Py_ssize_t size) {
  fail(0);
}

static PyObject *
P_get(void *ptr, Py_ssize_t size) {
  fail(0);
}

static PyObject *
O_set(void *ptr, PyObject *value, Py_ssize_t size) {
  fail(0);
}

static PyObject *
O_get(void *ptr, Py_ssize_t size) {
  fail(0);
}

static struct fielddesc formattable[] = {
  {'i', i_set, i_get, &ffi_type_sint, i_set_sw, i_get_sw},
  {'P', P_set, P_get, &ffi_type_pointer},
  {'O', O_set, O_get, &ffi_type_pointer},
  {0, NULL, NULL, NULL},
};

struct fielddesc *
_ctypes_get_fielddesc(const char *fmt) {
  static int initialized = 0;
  struct fielddesc *table = formattable;

  assert(fmt[0] != 'u'); // TODO miss some initialization for wchar_t. Follow cpy

  for (; table->code; ++table) {
    if (table->code == fmt[0]) {
      return table;
    }
  }

  fail("fielddesc not found for %c\n", fmt[0]);
  return NULL;
}
