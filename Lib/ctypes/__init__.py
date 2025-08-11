# TODO: properly implement these
c_double = 5

from _ctypes import RTLD_LOCAL
from _ctypes import dlopen as _dlopen
from _ctypes import CFuncPtr as _CFuncPtr
from _ctypes import _SimpleCData
from _ctypes import FUNCFLAG_CDECL as _FUNCFLAG_CDECL

DEFAULT_MODE = RTLD_LOCAL

class c_int(_SimpleCData):
    _type_ = "i"

class CDLL:

    def __init__(self, name):
        mode = DEFAULT_MODE
        handle = None

        self._name = name

        class _FuncPtr(_CFuncPtr):
            _restype_ = c_int
            _flags_ = _FUNCFLAG_CDECL

        self._FuncPtr = _FuncPtr

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
        func = self._FuncPtr((name_or_ordinal, self))
        if not isinstance(name_or_ordinal, int):
            func.__name__ = name_or_ordinal
        return func
