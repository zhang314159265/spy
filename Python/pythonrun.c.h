#pragma once

#include "pycore_parser.h"
#include "internal/pycore_compile.h"

PyObject * Py_CompileStringObject(const char *str, PyObject *filename, int start, PyCompilerFlags *flags, int optimize) {
  PyCodeObject *co;
  mod_ty mod;
  // TODO follow cpy
  PyArena *arena = NULL;
  
  mod = _PyParser_ASTFromString(str, filename, start, flags, arena);
  if (mod == NULL) {
    fail(0);
  }
  if (flags && (flags->cf_flags & PyCF_ONLY_AST)) {
    fail(0);
  }
  // co = _PyAST_Compile(mod, filename, flags, optimize, arena);
  co = _PyAST_Compile(mod);
  // _PyArena_Free(arena);
  return (PyObject *) co;
}
