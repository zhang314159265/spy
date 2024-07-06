ifeq ($(DBG), 1)
PREFIX := lldb --
else
PREFIX :=
endif

first: mine

all:
	DBG_TOK_GET=1 $(PREFIX) ../cpython/build/python.exe tutor.py

mine:
	gcc main.c -IInclude
	./a.out tutor.py
