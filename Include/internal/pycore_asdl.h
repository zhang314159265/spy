#ifndef Py_INTERNAL_ASDL_H
#define Py_INTERNAL_ASDL_H

#include "pyport.h"
#include "sutil.h"
#include "internal/pycore_pyarena.h"

typedef PyObject *identifier;
typedef PyObject *string;
typedef PyObject *object;
typedef PyObject *constant;

typedef struct {
	_ASDL_SEQ_HEAD
} asdl_seq;

typedef struct {
	_ASDL_SEQ_HEAD
	void *typed_elements[1];
} asdl_generic_seq;

#define GENERATE_ASDL_SEQ_CONSTRUCTOR(NAME, TYPE) \
asdl_ ## NAME ## _seq *_Py_asdl_ ## NAME ## _seq_new(Py_ssize_t size, PyArena *arena) \
{ \
	asdl_ ## NAME ## _seq *seq = NULL; \
	size_t n; \
	n = (size ? (sizeof(TYPE *) * (size - 1)) : 0); \
	n += sizeof(asdl_ ## NAME ## _seq); \
	seq = (asdl_ ## NAME ## _seq *)_PyArena_Malloc(arena, n); \
	assert(seq); \
	memset(seq, 0, n); \
	seq->size = size; \
	seq->elements = (void **) seq->typed_elements; \
	return seq; \
}

#define asdl_seq_SET_UNTYPED(S, I, V) (S)->elements[I] = (V)

#define asdl_seq_LEN(S) ((S) == NULL ? 0 : (S)->size)
#define asdl_seq_GET_UNTYPED(S, I) (S)->elements[(I)]
#define asdl_seq_GET(S, I) (S)->typed_elements[(I)]
#define asdl_seq_SET(S, I, V) (S)->typed_elements[I] = (V)

typedef struct {
	_ASDL_SEQ_HEAD
	int typed_elements[1];
} asdl_int_seq;

GENERATE_ASDL_SEQ_CONSTRUCTOR(generic, void*)
GENERATE_ASDL_SEQ_CONSTRUCTOR(int, int)
GENERATE_ASDL_SEQ_CONSTRUCTOR(expr, expr_ty)
GENERATE_ASDL_SEQ_CONSTRUCTOR(arg, arg_ty)

#endif
