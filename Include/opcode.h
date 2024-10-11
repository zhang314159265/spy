// TODO autogenerate this

#pragma once

#define POP_TOP 1
#define DUP_TOP 4
#define BINARY_ADD 23
#define INPLACE_ADD 55
#define GET_ITER 68
#define RETURN_VALUE 83
#define HAVE_ARGUMENT 90
#define STORE_NAME 90
#define DELETE_NAME 91
#define FOR_ITER 93
#define STORE_GLOBAL 97
#define DELETE_GLOBAL 98
#define LOAD_CONST 100
#define LOAD_NAME 101
#define JUMP_FORWARD 110
#define JUMP_IF_FALSE_OR_POP 111
#define JUMP_IF_TRUE_OR_POP 112
#define JUMP_ABSOLUTE 113
#define POP_JUMP_IF_FALSE 114
#define POP_JUMP_IF_TRUE 115
#define LOAD_GLOBAL 116
#define RERAISE 119
#define JUMP_IF_NOT_EXC_MATCH 121
#define SETUP_FINALLY 122
#define LOAD_FAST 124
#define STORE_FAST 125
#define DELETE_FAST 126
#define RAISE_VARARGS 130
#define CALL_FUNCTION 131
#define MAKE_FUNCTION 132
#define SETUP_WITH 143
#define SETUP_ASYNC_WITH 154

#define HAS_ARG(op) ((op) >= HAVE_ARGUMENT)
