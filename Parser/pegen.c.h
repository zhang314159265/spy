#pragma once

#include "Parser/pegen.h"

void _PyPegen_Parser_Free(Parser *p) {
  // Py_XDECREF(p->normalize);
  for (int i = 0; i < p->size; i++) {
    PyMem_Free(p->tokens[i]);
  }
  PyMem_Free(p->tokens);
  // growable_comment_array_deallocate
  PyMem_Free(p);
}

static int
compute_parser_flags(PyCompilerFlags *flags) {
  // TODO follow cpy
  return 0;
}

mod_ty
_PyPegen_run_parser_from_string(const char *str, int start_rule, PyObject *filename_ob, PyCompilerFlags *flags, PyArena *arena) {
  int exec_input = start_rule == Py_file_input;

  struct tok_state *tok;
  if (flags != NULL && flags->cf_flags & PyCF_IGNORE_COOKIE) {
    fail(0);
  } else {
    tok = PyTokenizer_FromString(str, exec_input);
  }
  if (tok == NULL) {
    fail(0);
  }
  tok->filename = filename_ob;
  Py_INCREF(filename_ob);

  mod_ty result = NULL;

  int parser_flags = compute_parser_flags(flags);
  int feature_version = flags && (flags->cf_flags & PyCF_ONLY_AST) ?
    flags->cf_feature_version : PY_MINOR_VERSION;

  #if 0 // TODO follow cpy
  Parser *p = _PyPegen_Parser_New(tok, start_rule, parser_flags, feature_version,
      NULL, arena);
  #else
  Parser *p = _PyPegen_Parser_New(tok);
  #endif
  if (p == NULL) {
    fail(0);
  }

  result = _PyPegen_run_parser(p);
  // dump_mod(result, 2);
  _PyPegen_Parser_Free(p);

error:
  PyTokenizer_Free(tok);
  return result;
}
