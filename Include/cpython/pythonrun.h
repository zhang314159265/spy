#include "internal/pycore_ast.h"
#include "internal/pycore_parser.h"
#include "internal/pycore_compile.h"
#include "dump_ast.h"
#include "eval.h"

static PyObject *
run_eval_code_obj(PyThreadState *tstate, PyCodeObject *co, PyObject *globals, PyObject *locals) {
	PyObject *v;

	v = PyEval_EvalCode((PyObject*) co, globals, locals);
	return v;
	assert(false);
}

// defined in cpy/Python/pythonrun.c
PyObject *
run_mod(mod_ty mod, PyObject *globals, PyObject *locals) {
	PyThreadState *tstate = _PyThreadState_GET();
	PyCodeObject *co = _PyAST_Compile(mod);
	if (co == NULL)
		return NULL;

	// call _PySys_Audit

	PyObject *v = run_eval_code_obj(tstate, co, globals, locals);
	Py_DECREF(co);
	return v;
	assert(false);
}

// This is not a public API in CPython
// Defined in cpy/Python/pythonrun.c
PyObject *pyrun_file(FILE *fp, PyObject *globals, PyObject *locals) {
	mod_ty mod;
	mod = _PyParser_ASTFromFile(fp);
	dump_mod(mod, 0);
	PyObject *ret;
	if (mod != NULL) {
		ret = run_mod(mod, globals, locals);
	} else {
		ret = NULL;
	}
	return ret;
}
