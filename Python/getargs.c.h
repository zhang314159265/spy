#pragma once

#include <stdarg.h>

#define FLAG_COMPAT 1

#define STATIC_FREELIST_ENTRIES 8

typedef struct {
} freelist_t;

static const char *
convertsimple(PyObject *arg, const char **p_format, va_list *p_va, int flags,
    char *msgbuf, size_t bufsize, freelist_t *freelist) {
  const char *format = *p_format;
  char c = *format++;

  switch (c) {
  case 'U': { // PyUnicode object
    PyObject **p = va_arg(*p_va, PyObject **);
    if (PyUnicode_Check(arg)) {
      if (PyUnicode_READY(arg) == -1)
        assert(false);
      *p = arg;
    } else {
      assert(false);
    }
    break;
  }
  case 'O': { // object
    PyTypeObject *type;
    PyObject **p;
    if (*format == '!') {
      type = va_arg(*p_va, PyTypeObject*);
      p = va_arg(*p_va, PyObject **);
      format++;
      if (PyType_IsSubtype(Py_TYPE(arg), type))
        *p = arg;
      else
        assert(false);
    } else if (*format == '&') {
      assert(false);
    } else {
      p = va_arg(*p_va, PyObject **);
      *p = arg;
    }
    break;
  }
  default:
    printf("convertsimple got char '%c'\n", c);
    assert(false);
  }

  *p_format = format;
  return NULL;
}

// Convert a single item
static const char *
convertitem(PyObject *arg, const char **p_format, va_list *p_va, int flags,
    int *levels, char *msgbuf, size_t bufsize, freelist_t *freelist) {
  const char *msg;
  const char *format = *p_format;

  if (*format == '(') {
    assert(false);
  } else {
    msg = convertsimple(arg, &format, p_va, flags,
        msgbuf, bufsize, freelist);
    if (msg != NULL)
      levels[0] = 0;
  }
  if (msg == NULL)
    *p_format = format;
  return msg;
}

static int
vgetargs1_impl(PyObject *compat_args, PyObject *const *stack, Py_ssize_t nargs, const char *format, va_list *p_va, int flags) {
  char msgbuf[256];
  int levels[32];
  const char *fname = NULL;
  const char *message = NULL;
  int min = -1;
  int max = 0;
  int level = 0;
  int endfmt = 0;
  const char *formatsave = format;
  Py_ssize_t i;
  const char *msg;
  int compat = flags & FLAG_COMPAT;
  freelist_t freelist;

  assert(nargs == 0 || stack != NULL);

  flags = flags & ~FLAG_COMPAT;

  while (endfmt == 0) {
    int c = *format++;
    switch (c) {
    case ':':
      message = format;
      endfmt = 1;
      break;
    case '|':
      if (level == 0)
        min = max;
      break;
    case '(':
    case ')':
    case '\0':
    case ';':
      printf("get char '%c'\n", c);
      assert(false);
    default:
      if (level == 0) {
        if (isalpha(c))
          if (c != 'e') // skip encoded
            max++;
      }
      break;
    }
  }

  if (level != 0) {
    assert(false);
  }

  if (min < 0)
    min = max;

  format = formatsave;

  if (max > STATIC_FREELIST_ENTRIES) {
    assert(false);
  }

  if (compat) {
    assert(false);
  }

  if (nargs < min || max < nargs) {
    assert(false);
  }

  for (i = 0; i < nargs; i++) {
    if (*format == '|')
      format++;

    msg = convertitem(stack[i], &format, p_va,
        flags, levels, msgbuf,
        sizeof(msgbuf), &freelist);
    if (msg) {
      assert(false);
    }
  }

  printf("format %s, max is %d, nargs %ld\n", formatsave, max, nargs);

  if (*format != '\0' && !isalpha(*format) &&
      *format != '(' &&
      *format != '|' && *format != ':' && *format != ';') {
    assert(false);
  }
  
  // TODO follow cpy to call
  // cleanreturn(1, &freelist);
  return 1;
}

static int
vgetargs1(PyObject *args, const char *format, va_list *p_va, int flags) {
  PyObject **stack;
  Py_ssize_t nargs;

  if (!(flags & FLAG_COMPAT)) {
    assert(args != NULL);

    if (!PyTuple_Check(args)) {
      assert(false);
    }

    stack = _PyTuple_ITEMS(args);
    nargs = PyTuple_GET_SIZE(args);
  } else {
    stack = NULL;
    nargs = 0;
  }

  return vgetargs1_impl(args, stack, nargs, format, p_va, flags);
}

int PyArg_ParseTuple(PyObject *args, const char *format, ...) {
  int retval;
  va_list va;

  va_start(va, format);
  retval = vgetargs1(args, format, &va, 0);
  va_end(va);
  return retval;
}

#undef _PyArg_NoKeywords

int
_PyArg_NoKeywords(const char *funcname, PyObject *kwargs) {
  assert(false);
}
