# TODO: properly implement these
c_double = 5

from _ctypes import RTLD_LOCAL
from _ctypes import dlopen as _dlopen

DEFAULT_MODE = RTLD_LOCAL

class CDLL:
    def __init__(self, name):
        mode = DEFAULT_MODE
        handle = None

        self._name = name

        if handle is None:
            self._handle = _dlopen(self._name, mode)
        else:
            self._handle = handle

    def __getattr__(self, name):
        assert False, "get {} attr from cdll".format(name)
