#include "object.h"

char * Py_UniversalNewlineFgets(char *buf, int n, FILE *stream, PyObject *fobj) {
	return fgets(buf, n, stream);
}
