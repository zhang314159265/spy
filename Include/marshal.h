#pragma once

#define FLAG_REF '\x80'

#define TYPE_CODE 'c'

typedef struct {
	FILE *fp;
	const char *ptr;
	const char *end;
	PyObject *refs; // a list
} RFILE;

static int
r_byte(RFILE *p) {
	int c = EOF;

	if (p->ptr != NULL) {
		if (p->ptr < p->end)
			c = (unsigned char) *p->ptr++;
		return c;
	}
	assert(false);
}

static PyObject *
r_object(RFILE *p) {
	PyObject *v;
	int type, code = r_byte(p);
	int flag;
	PyObject *retval = NULL;

	flag = code & FLAG_REF;
	type = code & ~FLAG_REF;

	switch (type) {
	case TYPE_CODE:
		{
			assert(false);
		}
		retval = v;
		break;
	default:
		printf("r_object type is %d ('%c')\n", type, (char)type);
		assert(false);
	}
	assert(false);
}

static PyObject *
read_object(RFILE *p) {
	PyObject *v;

	v = r_object(p);
	return v;
}

PyObject *
PyMarshal_ReadObjectFromString(const char *str, Py_ssize_t len) {
	RFILE rf;
	PyObject *result;

	rf.fp = NULL;
	rf.ptr = str;
	rf.end = str + len;
	rf.refs = PyList_New(0);
	if (rf.refs == NULL)
		return NULL;
	result = read_object(&rf);
	assert(false);
}
