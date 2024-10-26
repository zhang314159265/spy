Based on cpython 3.10.11 which is the default used on meta devgpu.

# NOTE
- run `make regen-pegen` to regenerated CPython parser.

# Archivement
- cpython 3.10.11 has 127 opcode. spy has implemented more than half. There are <=60 opcodes not implemented.

# Scratch

Quest 6:
- Class

Quest 5:
- closure

Quest x:
- generator / yield

Quest:
- decorators

Quest 1:
- cover import <+++++++++++

Quest 3:
- cover pickle ++++++++++

Quest 4:
- extension module ++++++++

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
- QUEST: explore the parser <++++
  - entry Parser/parser.c: `_PyPegen_parse`

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

## `run_mod`

`run_mod`: Python/pythonrun.c
- `_PyAST_Compile` # compile AST to bytecode
- `run_eval_code_obj`
  - `PyEval_EvalCode`: Python/ceval.c
    - `_PyEval_Vector`
      - `_PyEval_MakeFrameVector` # ref
      - `_PyEval_EvalFrame`: `Include/internal/pycore_ceval.h`
        - `_PyEval_EvalFrameDefault` # ref

## Parsing

`_PyParser_ASTFromFile`: `Parser/peg_api.c`
- `_PyPegen_run_parser_from_file_pointer`: Parser/pegen.c
  - `PyTokenizer_FromFile` # ref
  - `compute_parser_flags` # ref
  - `_PyPegen_Parser_New` # ref
  - `_PyPegen_run_parser` # ref

### Tokenizer

`struct tok_state`

`PyTokenizer_FromFile`: Parser/tokenizer.c . Create a `tok_state` with buffer allocated
- `tok_new` # create a new `struct tok_state`. Buffer is not allocated yet.

`PyTokenizer_Get`: Parser/tokenizer.c
- `tok_get`: Parser/tokenizer.c # get the next token.
  - `tok_nextc`: Parser/tokenizer.c # handle underflow in various input mode: string/interactive/file
    - `tok_underflow_string` # ref
    - `tok_underflow_file` # ref
    - `tok_underflow_interactive`
      - `translate_newlines` # handle \r \n stuff
      - `tok_reserve_buf`
  - `tok_backup` # put a character back to the buffer
  - `is_potential_identifier_start`

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

## AST to ByteCode

`struct compiler`: Python/compile.c

`_PyAST_Compile`: Python/compile.c # This function is the main entry point for this file.
- `compiler_init`: Python/compile.c # initialize 'struct compiler'
- `_PyFuture_FromAST`: Python/future.c
- `_PyAST_Optimize`: `Python/ast_opt.c` # TODO HERE <++++++
- `_PySymtable_Build`: Python/symtable.c # TODO HERE <+++++
- `compiler_mod`: Python/compile.c # TODO HERE <++++

## Code Object to Frame Object
`_PyEval_MakeFrameVector` # TODO

## Eval
`_PyEval_EvalFrameDefault`: Python/ceval.c # TODO HERE

# Done
- roughly understand the tokenizer. Haven't checked the codegen part for tokenizer in Parser/token.c though.

# old-Scratch
- how cpython do lexical and syntax analysis
- how does cprofile work
- how does the eval function work
- be familiar with bytecode
- how is example like `python -c 'print("hello")'` being handled
- find the print builtin
  - find file name using pattern `*bltin*`
  - search for print in the file and found the c function name `builtin_print`
  - set breakpoint at `builtin_print` and run a print statement in cpython. This will reveal stacktraces from main to calling `builtin_print`

## old-TODO
- Eval
- Parser code generation <++ TODO HERE
  - Parser/parser.c # generated code
  - python.gram # define the grammar
  - pegen.py generated Parser/parser.c from the grammar file
