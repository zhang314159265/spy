#include "internal/pycore_ast.h"
#include "internal/pycore_parser.h"
#include "dump_ast.h"

// This is not a public API in CPython
PyObject *pyrun_file(FILE *fp) {
	mod_ty mod;
	mod = _PyParser_ASTFromFile(fp);
	dump_mod(mod, 0);
	assert(false && "pyrun_file");
}
