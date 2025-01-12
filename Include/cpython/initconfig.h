#pragma once

typedef struct {
  enum {
    _PyStatus_TYPE_OK = 0,
    _PyStatus_TYPE_ERROR = 1,
    _PyStatus_TYPE_EXIT = 2,
  } _type;
  const char *func;
  const char *err_msg;
  int exitcode;
} PyStatus;

typedef struct PyConfig {
  wchar_t *check_hash_pycs_mode;
} PyConfig;

void _PyConfig_InitCompatConfig(PyConfig *config) {
  memset(config, 0, sizeof(*config));
  config->check_hash_pycs_mode = NULL;
}

static void
config_init_defaults(PyConfig *config) {
  _PyConfig_InitCompatConfig(config);
}

// defined in cpy/Python/initconfig.c
void PyConfig_InitPythonConfig(PyConfig *config) {
  config_init_defaults(config);
}

void PyConfig_Clear(PyConfig *config);
PyStatus PyConfig_SetString(PyConfig *config, wchar_t **config_str, const wchar_t *str);

PyStatus _PyConfig_Copy(PyConfig *config, const PyConfig *config2);
static PyStatus config_read(PyConfig *config, int compute_path_config);
PyStatus _PyConfig_Read(PyConfig *config, int compute_path_config);

void PyConfig_Clear(PyConfig *config) {
#define CLEAR(ATTR) \
  do { \
    PyMem_RawFree(ATTR); \
    ATTR = NULL; \
  } while (0)

  CLEAR(config->check_hash_pycs_mode);
}
