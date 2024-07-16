#pragma once

struct symtable {
};

struct symtable *
symtable_new(void) {
	struct symtable *st;
	st = (struct symtable *) PyMem_Malloc(sizeof(struct symtable));
	if (st == NULL) {
		assert(false);
	}
	return st;
}

// defined in cpy/Python/symtable.c
struct symtable *
_PySymtable_Build(struct _mod *mod) {
	struct symtable *st = symtable_new();
	if (st == NULL) {
		return NULL;
	}
	assert(false);
}
