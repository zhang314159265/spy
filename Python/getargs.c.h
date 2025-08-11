#pragma once

#include <stdarg.h>
#include "modsupport.h"

#define FLAG_COMPAT 1

#define STATIC_FREELIST_ENTRIES 8

#define IS_END_OF_FORMAT(c) (c == '\0' || c == ';' || c == ':')

typedef struct {
} freelistentry_t;

typedef struct {
  freelistentry_t *entries;
  int first_available;
  int entries_malloced;
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
      typedef int (*converter)(PyObject *, void *);
      converter convert = va_arg(*p_va, converter);
      void *addr = va_arg(*p_va, void *);
      int res;
      format++;
      if (!(res = (*convert)(arg, addr)))
        fail(0);
      if (res == Py_CLEANUP_SUPPORTED) {
        fail(0);
      }
    } else {
      p = va_arg(*p_va, PyObject **);
      *p = arg;
    }
    break;
  }
  case 'i': { // signed int
    int *p = va_arg(*p_va, int *);
    long ival = PyLong_AsLong(arg);
    if (ival == -1 && PyErr_Occurred())
      fail(0);
    else if (ival > INT_MAX) {
      fail(0);
    } else if (ival < INT_MIN) {
      fail(0);
    } else {
      *p = ival;
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
    case '\0':
      endfmt = 1;
      break;
    case ';':
      message = format;
      endfmt = 1;
      break;
    case '(':
    case ')':
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

  // printf("format %s, max is %d, nargs %ld\n", formatsave, max, nargs);

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
  if (kwargs == NULL) {
    return 1;
  }
  if (!PyDict_CheckExact(kwargs)) {
    fail(0);
  }
  if (PyDict_GET_SIZE(kwargs) == 0) {
    return 1;
  }

  fail("%s() takes no keyword arguments", funcname);

  #if 0
  PyErr_Format(PyExc_TypeError, "%s() takes no keyword arguments",
    fucnname);
  return 0;
  #endif
}

#undef _PyArg_CheckPositional

int
_PyArg_CheckPositional(const char *name, Py_ssize_t nargs,
    Py_ssize_t min, Py_ssize_t max) {
  assert(min >= 0);
  assert(min <= max);

  if (nargs < min) {
    assert(false);
  }

  if (nargs == 0) {
    return 1;
  }

  if (nargs > max) {
    assert(false);
  }

  return 1;
}

static int
unpack_stack(PyObject *const *args, Py_ssize_t nargs, const char *name,
    Py_ssize_t min, Py_ssize_t max, va_list vargs)
{
  Py_ssize_t i;
  PyObject **o;

  if (!_PyArg_CheckPositional(name, nargs, min, max)) {
    return 0;
  }

  for (i = 0; i < nargs; i++) {
    o = va_arg(vargs, PyObject **);
    *o = args[i];
  }
  return 1;
}

int PyArg_UnpackTuple(PyObject *args, const char *name, Py_ssize_t min, Py_ssize_t max, ...) {
  PyObject **stack;
  Py_ssize_t nargs;
  int retval;
  va_list vargs;

  if (!PyTuple_Check(args)) {
    assert(false);
  }
  stack = _PyTuple_ITEMS(args);
  nargs = PyTuple_GET_SIZE(args);

  va_start(vargs, max);
  retval = unpack_stack(stack, nargs, name, min, max, vargs);
  va_end(vargs);
  return retval;
}

static int
cleanreturn(int retval, freelist_t *freelist) {
  if (retval == 0) {
    // a failure ocurred
    assert(false);
  }
  if (freelist->entries_malloced) {
    PyMem_Free(freelist->entries);
  }
  return retval;
}

static int
vgetargskeywords(PyObject *args, PyObject *kwargs, const char *format,
    char **kwlist, va_list *p_va, int flags) {
  char msgbuf[512];
  int levels[32];
  const char *fname, *msg, *custom_msg;
  int min = INT_MAX;
  int max = INT_MAX;
  int i, pos, len;
  int skip = 0;
  Py_ssize_t nargs, nkwargs;
  PyObject *current_arg;
  freelistentry_t static_entries[STATIC_FREELIST_ENTRIES];
  freelist_t freelist;

  freelist.entries = static_entries;
  freelist.first_available = 0;
  freelist.entries_malloced = 0;

  fname = strchr(format, ':');
  if (fname) {
    fname++;
    custom_msg = NULL;
  } else {
    custom_msg = strchr(format, ';');
    if (custom_msg)
      custom_msg++;
  }

  for (pos = 0; kwlist[pos] && !*kwlist[pos]; pos++) {
  }
  for (len = pos; kwlist[len]; len++) {
    if (!*kwlist[len]) {
      assert(false);
    }
  }

  if (len > STATIC_FREELIST_ENTRIES) {
    assert(false);
  }

  nargs = PyTuple_GET_SIZE(args);
  nkwargs = (kwargs == NULL) ? 0 : PyDict_GET_SIZE(kwargs);
  if (nargs + nkwargs > len) {
    assert(false);
  }
  for (i = 0; i < len; i++) {
    if (*format == '|') {
      if (min != INT_MAX) {
        assert(false);
      }

      min = i;
      format++;

      if (max != INT_MAX) {
        assert(false);
      }
    }
    if (*format == '$') {
      if (max != INT_MAX) {
        assert(false);
      }

      max = i;
      format++;

      // printf("max %d, pos %d\n", max, pos);
    
      if (max < pos) {
        assert(false);
      }
      if (skip) {
        break;
      }
      if (max < nargs) {
        assert(false);
      }
    }
    if (IS_END_OF_FORMAT(*format)) {
      assert(false);
    }
    if (!skip) {
      if (i < nargs) {
        current_arg = PyTuple_GET_ITEM(args, i);
      } else if (nkwargs && i >= pos) {
        PyObject *_PyDict_GetItemStringWithError(PyObject *v, const char *key);
        current_arg = _PyDict_GetItemStringWithError(kwargs, kwlist[i]);
        if (current_arg) {
          --nkwargs;
        } else if (PyErr_Occurred()) {
          fail(0);
        }
      } else {
        current_arg = NULL;
      }

      if (current_arg) {
        msg = convertitem(current_arg, &format, p_va, flags,
            levels, msgbuf, sizeof(msgbuf), &freelist);
        if (msg) {
          fail(0);
        }
        continue;
      }

      if (i < min) {
        assert(false);
      }

      if (!nkwargs && !skip) {
        return cleanreturn(1, &freelist);
      }
    }
    assert(false);
  }

  if (skip) {
    fail(0);
  }

  if (!IS_END_OF_FORMAT(*format) && (*format != '|') && (*format != '$')) {
    fail(0);
  }

  if (nkwargs > 0) {
    fail(0);
  }

  return cleanreturn(1, &freelist);
}

int PyArg_ParseTupleAndKeywords(PyObject *args,
    PyObject *keywords,
    const char *format,
    char **kwlist, ...) {
  int retval;
  va_list va;

  if ((args == NULL || !PyTuple_Check(args)) ||
      (keywords != NULL && !PyDict_Check(keywords)) ||
      format == NULL ||
      kwlist == NULL)
  {
    assert(false);
  }

  va_start(va, kwlist);
  retval = vgetargskeywords(args, keywords, format, kwlist, &va, 0);
  va_end(va);
  return retval;
}

int _PyArg_NoKwnames(const char *funcname, PyObject *kwnames) {
  if (kwnames == NULL) {
    return 1;
  }

  assert(PyTuple_CheckExact(kwnames));

  if (PyTuple_GET_SIZE(kwnames) == 0) {
    return 1;
  }

  assert(false);
}

static struct _PyArg_Parser *static_arg_parsers = NULL;

struct _PyArg_Parser;
int parser_init(struct _PyArg_Parser *parser) {
  const char *const *keywords;
  const char *format, *msg;
  int i, len, min, max, nkw;
  PyObject *kwtuple;

  assert(parser->keywords != NULL);
  if (parser->kwtuple != NULL) {
    return 1;
  }

  keywords = parser->keywords;
  for (i = 0; keywords[i] && !*keywords[i]; i++) {
  }
  parser->pos = i;
  for (; keywords[i]; i++) {
    if (!*keywords[i]) {
      fail(0);
    }
  }
  len = i;

  format = parser->format;
  if (format) {
    fail(0);
  }

  nkw = len - parser->pos;
  kwtuple = PyTuple_New(nkw);
  if (kwtuple == NULL) {
    return 0;
  }
  keywords = parser->keywords + parser->pos;
  for (i = 0; i < nkw; i++) {
    PyObject *str = PyUnicode_FromString(keywords[i]);
    if (str == NULL) {
      fail(0);
    }
    PyUnicode_InternInPlace(&str);
    PyTuple_SET_ITEM(kwtuple, i, str);
  }
  parser->kwtuple = kwtuple;

  assert(parser->next == NULL);
  parser->next = static_arg_parsers;
  static_arg_parsers = parser;
  return 1;
}

PyObject *find_keyword(PyObject *kwnames, PyObject *const *kwstack, PyObject *key) {
  Py_ssize_t i, nkwargs;

  nkwargs = PyTuple_GET_SIZE(kwnames);

  for (i = 0; i < nkwargs; ++i) {
    PyObject *kwname = PyTuple_GET_ITEM(kwnames, i);

    if (kwname == key) {
      return kwstack[i];
    }
  }

  for (i = 0; i < nkwargs; ++i) {
    PyObject *kwname = PyTuple_GET_ITEM(kwnames, i);

    assert(PyUnicode_Check(kwname));
    if (_PyUnicode_EQ(kwname, key)) {
      return kwstack[i];
    }
  }
  return NULL;
}
