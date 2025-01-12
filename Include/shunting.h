#include <string.h>

#define fail(fmt, ...) do { \
  if (fmt) { \
    const char *_fmt = (const char *) fmt; \
    fprintf(stderr, _fmt __VA_OPT__(,) __VA_ARGS__); \
    if (*_fmt && _fmt[strlen(_fmt) - 1] != '\n') { \
      fprintf(stderr, "\n"); \
    } \
  } \
  assert(false); \
} while(0) 
