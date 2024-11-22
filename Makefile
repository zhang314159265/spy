ifeq ($(shell uname), Darwin)
DBG_PREFIX := lldb --
PYEXE := python.exe
else
DBG_PREFIX := gdb --args
PYEXE := python
endif

ifeq ($(DBG), 1)
PREFIX := $(DBG_PREFIX)
else
PREFIX :=
endif

TUTOR=tutor/misc.py
TUTOR=tutor/closure.py

first: mine
# first: pegen

PEGEN_FLAGS := -v

# CPY_DBG_FLAGS := DBG_TOK_GET=1
cpy:
	$(CPY_DBG_FLAGS) $(PREFIX) ../cpython/build/$(PYEXE) $(TUTOR)

pegen:
	PYTHONPATH=../cpython/Tools/peg_generator ../cpython/build/$(PYEXE) -m pegen $(PEGEN_FLAGS) -q c Grammar/python.gram Grammar/Tokens -o Parser/parser.c

CFLAGS := -IInclude -I.
mine: pegen
	gcc -g main.c $(CFLAGS)
	$(PREFIX) ./a.out $(TUTOR)
