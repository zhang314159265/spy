Based on cpython 3.10.11 which is the default used on meta devgpu.

# NOTE
- run `make regen-pegen` to regenerated CPython parser.

# Archivement
- cpython 3.10.11 has 127 opcode. spy has implemented more than half. There are <=60 opcodes not implemented.

# Future Features
- cprofile
- unit test framework
- c extension
- pickle
- exception handling
- write my own peg generator
  - Use CPython peg-generator first. Once it works end-2-end for the toy example, write a generator myself.
- Arguments when defining class (e.g. base class)



# Scratch

- import from the directory of the main script (import from current dir already works)

- create the frozon module for import myself (right now it's copied)

- import dataclasses ('import sum' already works)

- Move current code to legacy/
