#pragma once

#define FASTSEARCH ucs1lib_fastsearch
#define STRINGLIB(F) ucs1lib_##F
#define STRINGLIB_CHAR Py_UCS1
#define STRINGLIB_SIZEOF_CHAR 1
#define STRINGLIB_NEW _PyUnicode_FromUCS1
