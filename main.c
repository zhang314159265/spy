#include <stdio.h>
#include "shunting.h"
#include "object.h"
#include "Parser/tokenizer.h"
#include "token.h"
#include "cpython/pythonrun.h"
#include "internal/pycore_runtime.h"
#include "internal/pycore_pylifecycle.h"
#include "pylifecycle.h"
#include "typeobject.h"
#include "tupleobject.h"
#include "internal/pycore_pyerrors.h"

#include "allc.h"

#define DEBUG_TOKENIZER 0

static PyStatus pycore_create_interpreter(_PyRuntimeState *runtime, const PyConfig *config, PyThreadState **tstate_p) {
  PyInterpreterState *interp = PyInterpreterState_New();
  assert(interp);

  _PyConfig_Copy(&interp->config, config);

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
	PyObject *sysmod = NULL;

  status = pycore_init_singletons(interp);
  if (_PyStatus_EXCEPTION(status)) {
    return status;
  }

	status = pycore_init_types(interp);
	if (_PyStatus_EXCEPTION(status)) {
		assert(false);
	}

	status = _PySys_Create(tstate, &sysmod);
	if (_PyStatus_EXCEPTION(status)) {
		assert(false);
	}

	status = pycore_init_builtins(tstate);
	if (_PyStatus_EXCEPTION(status)) {
		assert(false);
	}

	if (init_importlib(tstate, sysmod) < 0) {
		assert(false);
	}
  return status;
}

static PyStatus pyinit_config(_PyRuntimeState *runtime, PyThreadState **tstate_p, const PyConfig *config) {
  PyStatus status;
  PyThreadState *tstate;
  pycore_create_interpreter(runtime, config, &tstate);
  *tstate_p = tstate;

  status = pycore_interp_init(tstate); // this will pre-create the list of small ints
  if (_PyStatus_EXCEPTION(status)) {
    return status;
  }

  runtime->core_initialized = 1;
  return _PyStatus_OK();
}

void pyinit_core(_PyRuntimeState *runtime, const PyConfig *src_config, PyThreadState **tstate_p) {
  PyStatus status;

  PyConfig config;
  PyConfig_InitPythonConfig(&config);

  status = _PyConfig_Copy(&config, src_config);
  if (_PyStatus_EXCEPTION(status)) {
    fail(0);
  }
  status = _PyConfig_Read(&config, 0);
  if (_PyStatus_EXCEPTION(status)) {
    fail(0);
  }
  pyinit_config(runtime, tstate_p, &config);

  PyConfig_Clear(&config);
}

// defined in cpy/Python/pylifecycle.c
void Py_InitializeFromConfig(const PyConfig *config) {
  _PyRuntimeState *runtime = &_PyRuntime;
  PyThreadState *tstate = NULL;
  pyinit_core(runtime, config, &tstate);
	pyinit_main(tstate);
}

void pymain_init() {
  _PyRuntime_Initialize();

  PyConfig config;
  PyConfig_InitPythonConfig(&config);

  Py_InitializeFromConfig(&config);
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
