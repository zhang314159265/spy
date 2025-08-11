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

# TUTOR=tutor/misc.py
TUTOR=tutor/use_ctypes.py

first: mine
# first: pegen

PEGEN_FLAGS := -v

EXTRA_ARGS :=

# CPY_DBG_FLAGS := DBG_TOK_GET=1
cpy:
	$(CPY_DBG_FLAGS) $(PREFIX) ../cpython/build/$(PYEXE) $(EXTRA_ARGS) $(TUTOR)

pegen:
	PYTHONPATH=../cpython/Tools/peg_generator ../cpython/build/$(PYEXE) -m pegen $(PEGEN_FLAGS) -q c Grammar/python.gram Grammar/Tokens -o Parser/parser.c

CFLAGS := -IInclude -I. -Wno-format-security
mine: pegen
	gcc -g main.c $(CFLAGS) -lm -lffi
	# cd tutor && $(PREFIX) ../a.out $(EXTRA_ARGS) basic_import.py # TODO
	$(PREFIX) ./a.out $(EXTRA_ARGS) $(TUTOR)
