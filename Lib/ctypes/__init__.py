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
        if name.startswith("__") and name.endswith("__"):
            assert False, "__getattr__ with dunder for CDLL"
        func = self.__getitem__(name)
        setattr(self, name, func)
        return func

    def __getitem__(self, name_or_ordinal):
        assert False, "CDLL.__getitem__"
