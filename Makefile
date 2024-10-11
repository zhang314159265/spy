ifeq ($(DBG), 1)
# TODO use lldb or gdb according to the presence of the tools
PREFIX := lldb --
# PREFIX := gdb --args
else
PREFIX :=
endif

TUTOR=tutor/sum_odd.py
TUTOR=tutor/sqrt.py

first: mine
# first: pegen

PEGEN_FLAGS := -v

# CPY_DBG_FLAGS := DBG_TOK_GET=1
cpy:
	$(CPY_DBG_FLAGS) $(PREFIX) ../cpython/build/python.exe $(TUTOR)

pegen:
	PYTHONPATH=../cpython/Tools/peg_generator ../cpython/build/python.exe -m pegen $(PEGEN_FLAGS) -q c Grammar/python.gram Grammar/Tokens -o Parser/parser.c

CFLAGS := -IInclude -I.
mine: pegen
	gcc -g main.c $(CFLAGS)
	$(PREFIX) ./a.out $(TUTOR)
