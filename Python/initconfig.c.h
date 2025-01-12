#pragma once

PyStatus
_PyConfig_Copy(PyConfig *config, const PyConfig *config2) {
  PyStatus status;

  PyConfig_Clear(config);

#define COPY_WSTR_ATTR(ATTR) \
  do { \
    status = PyConfig_SetString(config, &config->ATTR, config2->ATTR); \
    if (_PyStatus_EXCEPTION(status)) { \
      return status; \
    } \
  } while (0)


  COPY_WSTR_ATTR(check_hash_pycs_mode);
  return _PyStatus_OK();
}

PyStatus
PyConfig_SetString(PyConfig *config, wchar_t **config_str, const wchar_t *str) {
  PyStatus status = _Py_PreInitializeFromConfig(config, NULL);
  if (_PyStatus_EXCEPTION(status)) {
    return status;
  }

  wchar_t *str2;
  if (str != NULL) {
    str2 = _PyMem_RawWcsdup(str);
    if (str2 == NULL) {
      fail(0);
    }
  } else {
    str2 = NULL;
  }
  PyMem_RawFree(*config_str);
  *config_str = str2;
  return _PyStatus_OK();
}

static PyStatus
config_read(PyConfig *config, int compute_path_config) {
  PyStatus status;

  if (config->check_hash_pycs_mode == NULL) {
    status = PyConfig_SetString(config, &config->check_hash_pycs_mode, L"default");
    if (_PyStatus_EXCEPTION(status)) {
      return status;
    }
  }

  return _PyStatus_OK();
}

PyStatus
_PyConfig_Read(PyConfig *config, int compute_path_config) {
  PyStatus status;

  status = config_read(config, compute_path_config);
  if (_PyStatus_EXCEPTION(status)) {
    fail(0);
  }

  status = _PyStatus_OK();

  return status;
}


