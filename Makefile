ifeq ($(DBG), 1)
PREFIX := lldb --
else
PREFIX :=
endif

first: mine
# first: pegen

PEGEN_FLAGS := -v

pegen:
	PYTHONPATH=../cpython/Tools/peg_generator ../cpython/build/python.exe -m pegen $(PEGEN_FLAGS) -q c Grammar/python.gram Grammar/tokens -o Parser/parser.c

# CPY_DBG_FLAGS := DBG_TOK_GET=1
cpy:
	$(CPY_DBG_FLAGS) $(PREFIX) ../cpython/build/python.exe tutor.py

CFLAGS := -IInclude -I.
mine: pegen
# mine: 
	gcc -g main.c $(CFLAGS)
	$(PREFIX) ./a.out tutor.py
