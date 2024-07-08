#pragma once

struct _typeobject {
	PyObject_VAR_HEAD
	const char *tp_name; // for printing, in format "<module>.<name>"
	unsigned long tp_flags;
};
