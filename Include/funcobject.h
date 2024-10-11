#pragma once

#define COMMON_FIELDS(PREFIX) \
	PyObject *PREFIX ## globals; \
	PyObject *PREFIX ## builtins; \
	PyObject *PREFIX ## name; \
	PyObject *PREFIX ## qualname; \
	PyObject *PREFIX ## code; /* A code object, the __code__ attribute */ \
	PyObject *PREFIX ## defaults; \
	PyObject *PREFIX ## kwdefaults; \
	PyObject *PREFIX ## closure;

typedef struct {
	COMMON_FIELDS(fc_)
} PyFrameConstructor;
