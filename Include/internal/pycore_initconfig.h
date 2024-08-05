#pragma once

#define _PyStatus_OK() \
  (PyStatus){._type = _PyStatus_TYPE_OK,}

#define _PyStatus_EXCEPTION(err) \
  (err._type != _PyStatus_TYPE_OK)

#define _PyStatus_ERR(ERR_MSG) \
  (PyStatus) { \
    ._type = _PyStatus_TYPE_ERROR, \
    .func = __func__, \
    .err_msg = (ERR_MSG)}
