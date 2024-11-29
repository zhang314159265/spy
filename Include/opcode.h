// TODO autogenerate this

#pragma once

#define HAVE_ARGUMENT 90

enum {
#define DEF(name, val) name = val,
#include "opcode.inc"
#undef DEF
};

#define HAS_ARG(op) ((op) >= HAVE_ARGUMENT)

const char *opcode_to_str(int opcode) {
  switch (opcode) {
#define DEF(name, val) case name: return #name;
#include "opcode.inc"
#undef DEF
  default:
    printf("opcode_to_str: opcode %d\n", opcode);
    assert(false);
  }
}
