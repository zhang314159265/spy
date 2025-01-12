// #pragma once

#define FAST_COUNT 0
#define FAST_SEARCH 1
#define FAST_RSEARCH 2

Py_ssize_t
STRINGLIB(find_char)(const STRINGLIB_CHAR* s, Py_ssize_t n, STRINGLIB_CHAR ch) {
  // TODO follow cpy
  const STRINGLIB_CHAR *p, *e;

  p = s;
  e = s + n;
  while (p < e) {
    if (*p == ch)
      return (p - s);
    p++;
  }
  return -1;
}

Py_ssize_t
STRINGLIB(rfind_char)(const STRINGLIB_CHAR* s, Py_ssize_t n, STRINGLIB_CHAR ch) {
  const STRINGLIB_CHAR *p;
  p = s + n;
  while (p > s) {
    p--;
    if (*p == ch)
      return (p - s);
  }
  return -1;
}

Py_ssize_t
FASTSEARCH(const STRINGLIB_CHAR* s, Py_ssize_t n,
    const STRINGLIB_CHAR *p, Py_ssize_t m,
    Py_ssize_t maxcount, int mode) {
  Py_ssize_t count = 0;
  Py_ssize_t w;

  w = n - m;

  if (w < 0 || (mode == FAST_COUNT && maxcount == 0)) 
    return -1;

  if (m <= 1) {
    if (m <= 0)
      return -1;
    if (mode == FAST_SEARCH) {
      fail(0);
    } else if (mode == FAST_RSEARCH) {
      return STRINGLIB(rfind_char)(s, n, p[0]);
    } else {
      fail(0);
    }
  }

  if (mode != FAST_RSEARCH) {
    fail(0);
  } else {
    fail(0);
  }

  if (mode != FAST_COUNT)
    return -1;
  return count;
}
