#include <stdio.h>
#include "object.h"
#include "Parser/tokenizer.h"
#include "token.h"
#include "cpython/pythonrun.h"
#include "internal/pycore_runtime.h"
#include "internal/pycore_pylifecycle.h"
#include "pylifecycle.h"
#include "typeobject.h"
#include "tupleobject.h"

#include "allc.h"

#define DEBUG_TOKENIZER 0

static PyStatus pycore_create_interpreter(_PyRuntimeState *runtime, PyThreadState **tstate_p) {
  PyInterpreterState *interp = PyInterpreterState_New();
  assert(interp);

  PyThreadState *tstate = PyThreadState_New(interp);
  assert(tstate);
  (void) PyThreadState_Swap(tstate);

  // init_interp_create_gil

  *tstate_p = tstate;
  return _PyStatus_OK();
}

static PyStatus
pycore_init_singletons(PyInterpreterState *interp) {
  PyStatus status;

  if (_PyLong_Init(interp) < 0) {
    return _PyStatus_ERR("can't init longs");
  }
  return _PyStatus_OK();
}

static PyStatus
pycore_interp_init(PyThreadState *tstate) {
  PyInterpreterState *interp = tstate->interp;
  PyStatus status;

  status = pycore_init_singletons(interp);
  if (_PyStatus_EXCEPTION(status)) {
    return status;
  }

	status = pycore_init_types(interp);
	if (_PyStatus_EXCEPTION(status)) {
		assert(false);
	}
  return status;
}

static PyStatus pyinit_config(_PyRuntimeState *runtime, PyThreadState **tstate_p) {
  PyStatus status;
  PyThreadState *tstate;
  pycore_create_interpreter(runtime, &tstate);
  *tstate_p = tstate;

  status = pycore_interp_init(tstate); // this will pre-create the list of small ints
  if (_PyStatus_EXCEPTION(status)) {
    return status;
  }

  runtime->core_initialized = 1;
  return _PyStatus_OK();
}

void pyinit_core(_PyRuntimeState *runtime, PyThreadState **tstate_p) {
  pyinit_config(runtime, tstate_p);
}

// defined in cpy/Python/pylifecycle.c
void Py_InitializeFromConfig(void) {
  _PyRuntimeState *runtime = &_PyRuntime;
  PyThreadState *tstate = NULL;
  pyinit_core(runtime, &tstate);
  // TODO: use tstate to do more initialization. Call pyinit_main etc
}

void pymain_init() {
  _PyRuntime_Initialize();
  Py_InitializeFromConfig();
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage: %s PATH\n", argv[0]);
		return -1;
	}
	char *path = argv[1];
	FILE *fp = fopen(path, "r");
	assert(fp);

	#if DEBUG_TOKENIZER
	struct tok_state *tokenizer = PyTokenizer_FromFile(fp, NULL, NULL, NULL);
	int tok;
	while (1) {
		const char *start, *end;
		tok = tok_get(tokenizer, &start, &end);
		printf("tok is %d (%s): [%.*s]\n", tok, _get_token_name(tok), (int) (end - start), start);
		if (tok == ENDMARKER) {
			break;
		}
	}
	
	#endif

  pymain_init();
	PyObject *d = PyDict_New(); // TODO follow how cpy does
	pyrun_file(fp, d, d);

	fclose(fp);
	printf("bye\n");
	return 0;
}

#include "Parser/parser.c"
