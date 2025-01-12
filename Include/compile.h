#pragma once

#include "patchlevel.h"

// TODO follow cpy
// #define PyCF_MASK (0)

#define PyCF_SOURCE_IS_UTF8 0x0100
#define PyCF_ONLY_AST 0x0400
#define PyCF_IGNORE_COOKIE 0x0800

typedef struct {
  int cf_flags;
  int cf_feature_version;
} PyCompilerFlags;

#define _PyCompilerFlags_INIT \
  (PyCompilerFlags) {.cf_flags = 0, .cf_feature_version = PY_MINOR_VERSION}

#define Py_single_input 256
#define Py_file_input 257
#define Py_eval_input 258
#define Py_func_type_input 345
