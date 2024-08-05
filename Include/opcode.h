// TODO autogenerate this

#pragma once

#define POP_TOP 1
#define RETURN_VALUE 83
#define HAVE_ARGUMENT 90
#define STORE_NAME 90
#define DELETE_NAME 91
#define FOR_ITER 93
#define LOAD_CONST 100
#define LOAD_NAME 101
#define JUMP_FORWARD 110
#define JUMP_IF_FALSE_OR_POP 111
#define JUMP_IF_TRUE_OR_POP 112
#define JUMP_ABSOLUTE 113
#define POP_JUMP_IF_FALSE 114
#define POP_JUMP_IF_TRUE 115
#define RERAISE 119
#define RAISE_VARARGS 130
#define CALL_FUNCTION 131

#define HAS_ARG(op) ((op) >= HAVE_ARGUMENT)
