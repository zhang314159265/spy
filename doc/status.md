Based on cpython 3.10.11 which is the default used on meta devgpu.

# NOTE
- run `make regen-pegen` to regenerated CPython parser.

# Archivement
- cpython 3.10.11 has 127 opcode. spy has implemented more than half. There are <=60 opcodes not implemented.

# Scratch

Quest 1:
- cover import <+++++++++++

Quest 3:
- cover pickle ++++++++++

Quest 4:
- extension module ++++++++

Quest 5:
- cprofile

Later:
- Arguments when defining class (e.g. base class)

- goal
	- ? exception handling

- sys.stdout is defined by `init_sys_streams` 

`init_interp_main`
- `init_sys_streams`

## Read interprter initialization code
- `pymain_init`
  - `Py_InitializeFromConfig`
    - `pyinit_core`
      - `pyinit_config`
        - `pycore_create_interpreter`
          - `PyInterpreterState_New` <==
          - `PyThreadState_New`
          - `PyThreadState_Swap`
        - `pycore_interp_init` <==

- TODO: initialize the python runtime (for thread state and interpreter state)
    - call `_PyRuntime_Initialize` first
    - and then `Py_InitializeFromConfig` which calls `_PyGILState_SetTstate`

- QUEST: Use CPython peg-generator first. Once it works end-2-end for the toy example, write a generator myself.

# BELOW ARE OLD ################################################

# Dive into `print("hello")`

## General API

`Py_CHARMASK` # Include/pymacro.h. Mask out everything except the lowest byte.

## Main Entry Point

main: Programs/python.c
- `Py_BytesMain`: Modules/main.c
  - `pymain_main`: Modules/main.c
    - `pymain_init` # ref
  - `Py_RunMain`: Modules/main.c
    - `pymain_run_python`: Modules/main.c # support various ways to run python
      - `pymain_run_command` # ref
      - `pymain_run_module` # ref
      - `pymain_run_file` # ref
      - `pymain_run_stdin` # ref
      - `pymain_repl`: Modules/main.c # handle python '-i' argument

## Run from stdin
`pymain_run_stdin`: Modules/main.c
- `pymain_run_startup` # run startup script if needed
  - `PyRun_AnyFileExFlags`: Python/pythonrun.c
    - `_PyRun_AnyFileObject`: Python/pythonrun.c
      - `_PyRun_SimpleFileObject` # ref
      - `_PyRun_InteractiveLoopObject`: Python/pythonrun.c
        - `PyRun_InteractiveOneObjectEx`
          - `_PyArena_New` # ref
          - `_PyParser_ASTFromFile` # ref
          - `PyImport_AddModuleObject` # ref
          - `run_mod` # ref

### Parser

`typedef Token`: Parser/pegen.h # not sure how is memo used yet

`typedef Parser`: Parser/pegen.h

`_PyPegen_Parser_New`: Parser/pegen.c # done

`_PyPegen_run_parser`: Parser/pegen.c
- `_PyPegen_parse`: Parser/parser.c (gen) # TODO HERE

`_PyPegen_expect_token`: Parser/pegen.c
- `_PyPegen_fill_token`: Parser/pegen.c
  - `PyTokenizer_Get` # ref
  - `_resize_tokens_array`
  - `initialize_token` # roughly done. Didn't figure out the handling for `col_offset` and `end_col_offset` yet.
    - `_get_keyword_or_name_type` # distinguish keywords with identifiers/names

`Grammar/python.gram` # VERY roughtly went thru

## Code Object to Frame Object
`_PyEval_MakeFrameVector` # TODO

# old-Scratch
- how does cprofile work
- find the print builtin
  - find file name using pattern `*bltin*`
  - search for print in the file and found the c function name `builtin_print`
  - set breakpoint at `builtin_print` and run a print statement in cpython. This will reveal stacktraces from main to calling `builtin_print`

