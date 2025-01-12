#pragma once

mod_ty _PyParser_ASTFromString(const char *str, PyObject *filename, int mode, PyCompilerFlags *flags, PyArena *arena) {
  mod_ty result = _PyPegen_run_parser_from_string(str, mode, filename, flags, arena);
  return result;
}
