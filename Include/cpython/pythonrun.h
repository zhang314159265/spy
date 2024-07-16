#include "internal/pycore_ast.h"
#include "internal/pycore_parser.h"
#include "internal/pycore_compile.h"
#include "dump_ast.h"

PyObject *
run_mod(mod_ty mod) {
	PyCodeObject *co = _PyAST_Compile(mod);
	assert(false);
}

// This is not a public API in CPython
// Defined in cpy/Python/pythonrun.c
PyObject *pyrun_file(FILE *fp) {
	mod_ty mod;
	mod = _PyParser_ASTFromFile(fp);
	dump_mod(mod, 0);
	PyObject *ret;
	if (mod != NULL) {
		ret = run_mod(mod);
	} else {
		ret = NULL;
	}
	return ret;
}
