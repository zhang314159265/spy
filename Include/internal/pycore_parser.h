#ifndef Py_INTERNAL_PARSER_H
#define Py_INTERNAL_PARSER_H

#include "Parser/pegen.h"

// defined in cpy/Parser/peg_api.c
struct _mod* _PyParser_ASTFromFile(FILE *fp) {
	return _PyPegen_run_parser_from_file_pointer(fp);
}

#endif
