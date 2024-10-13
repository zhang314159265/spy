#pragma once

PyDoc_STRVAR(list_append__doc__, "");

#define LIST_APPEND_METHODDEF \
	{"append", (PyCFunction) list_append, METH_O, list_append__doc__},
