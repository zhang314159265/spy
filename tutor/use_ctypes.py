import ctypes
from ctypes import c_double

libc = ctypes.CDLL("libc.so.6")
libc.srand(5)

EXPECTED_FOR_SEED_5 = 590011675  # output of this c file: https://gist.github.com/zhang314159265/fca088a3c1d767d961691dfb347caa71 on a linux machine.
print("Received the correct random number?", "yes" if libc.rand() == EXPECTED_FOR_SEED_5 else "no")

libm = ctypes.CDLL("libm.so.6")
sqrt = libm.sqrt
sqrt.restype = c_double
print("sqrt of 2.0 is", sqrt(c_double(2.0)))
