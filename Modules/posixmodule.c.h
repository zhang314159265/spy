#pragma once

#define INITFUNC PyInit_posix
#define MODNAME "posix"

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#define DEFAULT_DIR_FD (-100)

typedef struct {
  PyObject *billion;
  PyObject *StatResultType;
} _posixstate;



static PyObject *
posix_getcwd(int use_bytes) {
  const size_t chunk = 1024;
  char *buf = NULL;
  char *cwd = NULL;
  size_t buflen = 0;

  do {
    char *newbuf;
    if (buflen <= PY_SSIZE_T_MAX - chunk) {
      buflen += chunk;
      newbuf = PyMem_RawRealloc(buf, buflen);
    } else {
      newbuf = NULL;
    }
    if (newbuf == NULL) {
      fail(0);
    }
    buf = newbuf;

    cwd = getcwd(buf, buflen);
  } while (cwd == NULL && errno == ERANGE);

  if (buf == NULL) {
    fail(0);
  }

  if (cwd == NULL) {
    fail(0);
  }

  PyObject *obj;
  if (use_bytes) {
    fail(0);
  } else {
    obj = PyUnicode_DecodeFSDefault(buf);
  }
  PyMem_RawFree(buf);
  return obj;
}

static PyObject *
os_getcwd_impl(PyObject *module) {
  return posix_getcwd(0);
}

static PyObject *
os_getcwd(PyObject *module, PyObject *ignored) {
  return os_getcwd_impl(module);
}

#define OS_GETCWD_METHODDEF \
  {"getcwd", (PyCFunction) os_getcwd, METH_NOARGS, ""},

typedef struct {
  const char *function_name;
  const char *argument_name;
  int nullable;
  int allow_fd;
  const wchar_t *wide;
  const char *narrow;
  int fd;
  Py_ssize_t length;
  PyObject *object;
  PyObject *cleanup;
} path_t;

#define PATH_T_INITIALIZE(function_name, argument_name, nullable, allow_fd) \
  {function_name, argument_name, nullable, allow_fd, NULL, NULL, -1, 0, NULL, NULL}

static void
path_cleanup(path_t *path) {
  wchar_t *wide = (wchar_t *) path->wide;
  path->wide = NULL;
  PyMem_Free(wide);
  Py_CLEAR(path->object);
  Py_CLEAR(path->cleanup);
}

static int
path_converter(PyObject *o, void *p) {
  path_t *path = (path_t *) p;
  PyObject *bytes = NULL;
  Py_ssize_t length = 0;
  int is_index, is_buffer, is_bytes, is_unicode;
  const char *narrow;

  if (o == NULL) {
    fail(0);
  }

  path->object = path->cleanup = NULL;
  Py_INCREF(o);

  if ((o == Py_None) && path->nullable) {
    fail(0);
  }

  is_index = path->allow_fd && PyIndex_Check(o);
  is_buffer = PyObject_CheckBuffer(o);
  is_bytes = PyBytes_Check(o);
  is_unicode = PyUnicode_Check(o);

  if (!is_index && !is_buffer && !is_unicode && !is_bytes) {
    fail(0);
  }
  if (is_unicode) {
    if (!PyUnicode_FSConverter(o, &bytes)) {
      fail(0);
    }
  } else {
    fail(0);
  }
  length = PyBytes_GET_SIZE(bytes);
  narrow = PyBytes_AS_STRING(bytes);
  if ((size_t) length != strlen(narrow)) {
    fail(0);
  }

  path->wide = NULL;
  path->narrow = narrow;
  if (bytes == o) {
    Py_DECREF(bytes);
  } else {
    path->cleanup = bytes;
  }
  path->fd = -1;

  path->length = length;
  path->object = o;
  return Py_CLEANUP_SUPPORTED;
}

#define STRUCT_STAT struct stat
#define STAT stat

static inline _posixstate *
get_posix_state(PyObject *module) {
  void *state = _PyModule_GetState(module);
  assert(state != NULL);
  return (_posixstate *) state;
}

#define _PyLong_FromDev PyLong_FromLongLong

PyObject *
_PyLong_FromUid(uid_t uid) {
  if (uid == (uid_t) -1) {
    return PyLong_FromLong(-1);
  }
  return PyLong_FromUnsignedLong(uid);
}

PyObject *
_PyLong_FromGid(gid_t gid) {
  if (gid == (gid_t) -1) {
    return PyLong_FromLong(-1);
  }
  return PyLong_FromUnsignedLong(gid);
}

static void
fill_time(PyObject *module, PyObject *v, int index, time_t sec, unsigned long nsec) {
  #if 0
  PyObject *s = _PyLong_FromTime_t(sec);
  PyObject *ns_fractional = PyLong_FromUnsignedLong(nsec);
  PyObject *s_in_ns = NULL;
  PyObject *ns_total = NULL;
  PyObject *float_s = NULL;

  if (!(s && ns_fractional))
    fail(0);

  s_in_ns = PyNumber_Multiply(s, get_posix_state(module)->billion);
  if (!s_in_ns) {
    fail(0);
  }
  fail(0);
  #else

  PyObject *float_s = NULL;
  float_s = PyFloat_FromDouble(sec + 1e-9 * nsec);
  if (!float_s) {
    fail(0);
  }
  PyStructSequence_SET_ITEM(v, index, float_s);
  float_s = NULL;

  Py_XDECREF(float_s);
  
  #endif
}

static PyObject *
_pystat_fromstructstat(PyObject *module, STRUCT_STAT *st) {
  unsigned long ansec, mnsec, cnsec;
  PyObject *StatResultType = get_posix_state(module)->StatResultType;
  PyObject *v = PyStructSequence_New((PyTypeObject *) StatResultType);
  if (v == NULL)
    return NULL;

  PyStructSequence_SET_ITEM(v, 0, PyLong_FromLong((long) st->st_mode));
  assert(sizeof(unsigned long long) >= sizeof(st->st_ino));
  PyStructSequence_SET_ITEM(v, 1, PyLong_FromUnsignedLongLong(st->st_ino));
  PyStructSequence_SET_ITEM(v, 2, _PyLong_FromDev(st->st_dev));
  PyStructSequence_SET_ITEM(v, 3, PyLong_FromLong((long) st->st_nlink));

  PyStructSequence_SET_ITEM(v, 4, _PyLong_FromUid(st->st_uid));
  PyStructSequence_SET_ITEM(v, 5, _PyLong_FromGid(st->st_gid));
  assert(sizeof(long long) >= sizeof(st->st_size));
  PyStructSequence_SET_ITEM(v, 6, PyLong_FromLongLong(st->st_size));

  #if 0
  // only works in some of the platforms
  ansec = st->st_atimespec.tv_nsec;
  mnsec = st->st_mtimespec.tv_nsec;
  cnsec = st->st_ctimespec.tv_nsec;
  #else
  ansec = mnsec = cnsec = 0;
  #endif

  fill_time(module, v, 7, st->st_atime, ansec);
  fill_time(module, v, 8, st->st_mtime, mnsec);
  fill_time(module, v, 9, st->st_ctime, cnsec);

  if (PyErr_Occurred()) {
    Py_DECREF(v);
    return NULL;
  }
  return v;
}

static PyObject *
posix_path_object_error(PyObject *path) {
  return PyErr_SetFromErrnoWithFilenameObject(PyExc_OSError, path);
}

static PyObject *
path_object_error(PyObject *path) {
  return posix_path_object_error(path);
}

static PyObject *
path_error(path_t *path) {
  return path_object_error(path->object);
}

static PyObject *
posix_do_stat(PyObject *module, const char *function_name, path_t *path,
    int dir_fd, int follow_symlinks) {
  STRUCT_STAT st;
  int result;

  int fstatat_unavailable = 0;

  #if 0
  if (path_and_dir_fd_invalid("stat", path, dir_fd) ||
    dir_fd_and_fd_invalid("stat", dir_fd, path->fd) ||
    fd_and_fllow_symlinks_invalid("stat", path->fd, follow_symlinks))
    return NULL;
  #endif

  if (path->fd != -1) {
    fail(0);
  } else if ((dir_fd != DEFAULT_DIR_FD) || !follow_symlinks) {
    fail(0);
  } else {
    result = STAT(path->narrow, &st);
  }

  if (fstatat_unavailable) {
    fail(0);
  }
  
  if (result != 0) {
    return path_error(path);
  }

  return _pystat_fromstructstat(module, &st);
}

static PyObject *
os_stat_impl(PyObject *module, path_t *path, int dir_fd, int follow_symlinks) {
  return posix_do_stat(module, "stat", path, dir_fd, follow_symlinks);
}

static PyObject *
os_stat(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames) {
  PyObject *return_value = NULL;
  static const char *const _keywords[] = {"path", "dir_fd", "follow_symlinks", NULL};
  static _PyArg_Parser _parser = {NULL, _keywords, "stat", 0};
  PyObject *argsbuf[3];
  Py_ssize_t noptargs = nargs + (kwnames ? PyTuple_GET_SIZE(kwnames) : 0) - 1;
  path_t path = PATH_T_INITIALIZE("stat", "path", 0, 1);
  int dir_fd = DEFAULT_DIR_FD;
  int follow_symlinks = 1;

  args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser, 1, 1, 0, argsbuf);
  if (!args) {
    fail(0);
  }
  if (!path_converter(args[0], &path)) {
    fail(0);
  }
  if (!noptargs) {
    goto skip_optional_kwonly;
  }
  if (args[1]) {
    fail(0);
  }
  follow_symlinks = PyObject_IsTrue(args[2]);
  if (follow_symlinks < 0) {
    fail(0);
  }
skip_optional_kwonly:
  return_value = os_stat_impl(module, &path, dir_fd, follow_symlinks);

exit:
  path_cleanup(&path);
  return return_value;
}

#define OS_STAT_METHODDEF \
  {"stat", (PyCFunction) (void(*)(void)) os_stat, METH_FASTCALL | METH_KEYWORDS, ""},

#define OS_LISTDIR_METHODDEF \
  {"listdir", (PyCFunction)(void(*)(void)) os_listdir, METH_FASTCALL | METH_KEYWORDS, ""},

#define NAMLEN(dirent) strlen((dirent)->d_name)

static PyObject *
_posix_listdir(path_t *path, PyObject *list) {
  PyObject *v;
  DIR *dirp = NULL;
  struct dirent *ep;
  int return_str;
  int fd = -1;

  errno = 0;
  if (path->fd != -1) {
    fail(0);
  } else {
    const char *name;
    if (path->narrow) {
      name = path->narrow;
      return_str = !PyObject_CheckBuffer(path->object);
    } else {
      name = ".";
      return_str = 1;
    }
    dirp = opendir(name);
  }

  if (dirp == NULL) {
    fail(0);
  }
  if ((list = PyList_New(0)) == NULL) {
    fail(0);
  }
  for (;;) {
    errno = 0;
    ep = readdir(dirp);
    if (ep == NULL) {
      if (errno == 0) {
        break;
      } else {
        fail(0);
      }
    }
    if (ep->d_name[0] == '.' &&
      (NAMLEN(ep) == 1 ||
        (ep->d_name[1] == '.' && NAMLEN(ep) == 2)))
      continue;
    if (return_str)
      v = PyUnicode_DecodeFSDefaultAndSize(ep->d_name, NAMLEN(ep));
    else
      fail(0);
    if (v == NULL) {
      Py_CLEAR(list);
      break;
    }
    if (PyList_Append(list, v) != 0) {
      fail(0);
    }
    Py_DECREF(v);
  }
  if (dirp != NULL) {
    if (fd > -1) {
      fail(0);
    }
    closedir(dirp);
  }
  return list;
}

static PyObject *
os_listdir_impl(PyObject *module, path_t *path) {
  return _posix_listdir(path, NULL);
}

#define PATH_HAVE_FDOPENDIR 1

static PyObject *
os_listdir(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames) {
  PyObject *return_value = NULL;
  static const char *const _keywords[] = {"path", NULL};
  static _PyArg_Parser _parser = {NULL, _keywords, "listdir", 0};
  PyObject *argsbuf[1];
  Py_ssize_t noptargs = nargs + (kwnames ? PyTuple_GET_SIZE(kwnames) : 0) - 0;
  path_t path = PATH_T_INITIALIZE("listdir", "path", 1, PATH_HAVE_FDOPENDIR);
  args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser, 0, 1, 0, argsbuf);
  if (!args) {
    goto exit;
  }
  if (!noptargs) {
    goto skip_optional_pos;
  }
  if (!path_converter(args[0], &path)) {
    goto exit;
  }
skip_optional_pos:
  return_value = os_listdir_impl(module, &path);
exit:
  path_cleanup(&path);

  return return_value;
}

#define OS_FSPATH_METHODDEF \
  {"fspath", (PyCFunction)(void(*)(void)) os_fspath, METH_FASTCALL | METH_KEYWORDS, ""},

static PyObject *
os_fspath_impl(PyObject *module, PyObject *path) {
  return PyOS_FSPath(path);
}

static PyObject *
os_fspath(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames) {
  PyObject *return_value = NULL;
  static const char *const _keywords[] = {"path", NULL};
  static _PyArg_Parser _parser = {NULL, _keywords, "fspath", 0};
  PyObject *argsbuf[1];
  PyObject *path;

  args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser, 1, 1, 0, argsbuf);
  if (!args) {
    goto exit;
  }
  path = args[0];
  return_value = os_fspath_impl(module, path);

exit:
  return return_value;
}

static PyMethodDef posix_methods[] = {
  OS_STAT_METHODDEF
  OS_GETCWD_METHODDEF
  OS_LISTDIR_METHODDEF
  OS_FSPATH_METHODDEF
  {NULL, NULL},
};

static int
_posix_clear(PyObject *module) {
  fail(0);
}

static void
_posix_free(void *module) {
  fail(0);
}

static PyStructSequence_Field stat_result_fields[] = {
  {"st_mode", "protection bits"},
  {"st_ino", "inode"},
  {"st_dev", "device"},
  {"st_nlink", "number of hard links"},
  {"st_uid", "user ID of owner"},
  {"st_gid", "group ID of owner"},
  {"st_size", "total size, in bytes"},
  #if 0 // TODO handle null fields
  {NULL, "integer time of last access"},
  {NULL, "integer time of last modification"},
  {NULL, "integer time of last change"},
  #endif
  {"st_atime", "time of last access"},
  {"st_mtime", "time of last modification"},
  {"st_ctime", "time of last change"},
  {0},
};

static PyStructSequence_Desc stat_result_desc = {
  .name = "stat_result",
  .doc = "",
  .fields = stat_result_fields,
  .n_in_sequence = 10,
};

// (shunting) rename from structseq_new to avoid name collision
static newfunc _posixmodule_structseq_new;

static PyObject *
statresult_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  PyStructSequence *result;
  int i;

  result = (PyStructSequence *) _posixmodule_structseq_new(type, args, kwds);
  if (!result)
    return NULL;
  for (i = 7; i <= 9; i++) {
    if (result->ob_item[i + 3] == Py_None) {
      Py_DECREF(Py_None);
      Py_INCREF(result->ob_item[i]);
      result->ob_item[i + 3] = result->ob_item[i];
    }
  }
  return (PyObject *) result;
}

static PyObject *
convertenviron() {
  // TODO populate the kv pairs
  PyObject *d;

  d = PyDict_New();
  if (d == NULL)
    return NULL;
  return d;
}

static int
posixmodule_exec(PyObject *m) {
  _posixstate *state = get_posix_state(m);

  // Initialize environ dictionary
  PyObject *v = convertenviron();
  Py_XINCREF(v);
  if (v == NULL || PyModule_AddObject(m, "environ", v) != 0)
    return -1;
  Py_DECREF(v);

  PyObject *StatResultType = (PyObject *) PyStructSequence_NewType(&stat_result_desc);
  if (StatResultType == NULL) {
    return -1;
  }
  Py_INCREF(StatResultType);
  PyModule_AddObject(m, "stat_result", StatResultType);
  state->StatResultType = StatResultType;
  _posixmodule_structseq_new = ((PyTypeObject *)StatResultType)->tp_new;
  ((PyTypeObject *) StatResultType)->tp_new = statresult_new;

  if ((state->billion = PyLong_FromLong(1000000000)) == NULL)
    return -1;

  return 0;
}

static PyModuleDef_Slot posixmodule_slots[] = {
  {Py_mod_exec, posixmodule_exec},
  {0, NULL},
};

static struct PyModuleDef posixmodule = {
  PyModuleDef_HEAD_INIT,
  .m_name = MODNAME,
  .m_doc = "",
  .m_size = sizeof(_posixstate),
  .m_methods = posix_methods,
  .m_slots = posixmodule_slots,
  .m_clear = _posix_clear,
  .m_free = _posix_free,
};

PyMODINIT_FUNC
INITFUNC(void) {
  return PyModuleDef_Init(&posixmodule);
}

PyObject *PyOS_FSPath(PyObject *path) {
  if (PyUnicode_Check(path) || PyBytes_Check(path)) {
    Py_INCREF(path);
    return path;
  }
  fail(0);
}
