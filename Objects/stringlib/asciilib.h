#pragma once

#define FASTSEARCH asciilib_fastsearch
#define STRINGLIB(F) asciilib_##F
#define STRINGLIB_CHAR Py_UCS1
#define STRINGLIB_SIZEOF_CHAR 1
#define STRINGLIB_NEW(STR, LEN) _PyUnicode_FromASCII((const char *) (STR), (LEN))
