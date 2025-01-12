#pragma once

typedef struct {
	PyObject *str;
	Py_ssize_t start, end;
} SubString;

typedef enum {
	ANS_INIT,
	ANS_AUTO,
	ANS_MANUAL,
} AutoNumberState;

typedef struct {
	AutoNumberState an_state;
	int an_field_number;
} AutoNumber;

static void
AutoNumber_Init(AutoNumber *auto_number) {
	auto_number->an_state = ANS_INIT;
	auto_number->an_field_number = 0;
}

void
SubString_init(SubString *str, PyObject *s, Py_ssize_t start, Py_ssize_t end) {
	str->str = s;
	str->start = start;
	str->end = end;
}

typedef struct {
	SubString str;
} MarkupIterator;

static int
MarkupIterator_init(MarkupIterator *self, PyObject *str,
		Py_ssize_t start, Py_ssize_t end) {
	SubString_init(&self->str, str, start, end);
	return 1;
}

static int
parse_field(SubString *str, SubString *field_name, SubString *format_spec,
		int *format_spec_needs_expanding, Py_UCS4 *conversion) {
	// printf("char %c\n", PyUnicode_READ_CHAR(str->str, str->start));
	Py_UCS4 c = 0;

	*conversion = '\0';
	SubString_init(format_spec, NULL, 0, 0);

	// search for the field name
	field_name->str = str->str;
	field_name->start = str->start;
	while (str->start < str->end) {
		switch ((c = PyUnicode_READ_CHAR(str->str, str->start++))) {
		case '{':
			assert(false);
		case '[':
			assert(false);
		case '}':
		case ':':
		case '!':
			break;
		default:
			continue;
		}
		break;
	}

	field_name->end = str->start - 1;

	if (c == '!' || c == ':') {
    Py_ssize_t count;

    if (c == '!') {
      if (str->start >= str->end) {
        fail(0);
      }
      *conversion = PyUnicode_READ_CHAR(str->str, str->start++);

      if (str->start < str->end) {
        c = PyUnicode_READ_CHAR(str->str, str->start++);
        if (c == '}')
          return 1;
        if (c != ':') {
          fail(0);
        }
      }
    }
		assert(false);
	} else if (c != '}') {
		assert(false);
	}

	return 1;
}

// returns 0 on error, 1 on non-error termination, and 2 if it got a
// string (or something to be expanded)
static int
MarkupIterator_next(MarkupIterator *self, SubString *literal,
		int *field_present, SubString *field_name,
		SubString *format_spec, Py_UCS4 *conversion,
		int *format_spec_needs_expanding) {
	int at_end;
	Py_UCS4 c = 0;
	Py_ssize_t start;
	Py_ssize_t len;
	int markup_follows = 0;

	// Initialize all of the output variables
	SubString_init(literal, NULL, 0, 0);
	SubString_init(field_name, NULL, 0, 0);
	SubString_init(format_spec, NULL, 0, 0);
	*conversion = '\0';
	*format_spec_needs_expanding = 0;
	*field_present = 0;

	if (self->str.start >= self->str.end)
		return 1;
	
	start = self->str.start;
	while (self->str.start < self->str.end) {
		switch (c = PyUnicode_READ_CHAR(self->str.str, self->str.start++)) {
		case '{':
		case '}':
			markup_follows = 1;
			break;
		default:
			continue;
		}
		break;
	}

	at_end = self->str.start >= self->str.end;
	len = self->str.start - start;

	if (!at_end) {
		// don't handle escaping of '{' '}' yet
		len--;
	}

	// record the literal text
	literal->str = self->str.str;
	literal->start = start;
	literal->end = start + len;

	if (!markup_follows)
		return 2;

	// this is markup; parse the field
	*field_present = 1;
	if (!parse_field(&self->str, field_name, format_spec,
			format_spec_needs_expanding, conversion))
		return 0;
	return 2;
}

typedef struct {
	SubString str;
	Py_ssize_t index;
} FieldNameIterator;

static int
FieldNameIterator_init(FieldNameIterator *self, PyObject *s,
		Py_ssize_t start, Py_ssize_t end) {
	SubString_init(&self->str, s, start, end);
	self->index = start;
	return 1;
}

static Py_ssize_t
get_integer(const SubString *str) {
	if (str->start >= str->end)
		return -1;
	assert(false);
}

static int
field_name_split(PyObject *str, Py_ssize_t start, Py_ssize_t end, SubString *first,
		Py_ssize_t *first_idx, FieldNameIterator *rest,
		AutoNumber *auto_number) {
	Py_UCS4 c;
	Py_ssize_t i = start;
	int field_name_is_empty;
	int using_numeric_index;

	while (i < end) {
		switch (c = PyUnicode_READ_CHAR(str, i++)) {
		case '[':
		case '.':
			i--;
			break;
		default:
			continue;
		}
		break;
	}

	// set up the return values
	SubString_init(first, str, start, i);
	FieldNameIterator_init(rest, str, i, end);

	*first_idx = get_integer(first);
	if (*first_idx == -1 && PyErr_Occurred())
		return 0;
	
	field_name_is_empty = first->start >= first->end;

	using_numeric_index = field_name_is_empty || *first_idx != -1;

	if (auto_number) {
		if (auto_number->an_state == ANS_INIT && using_numeric_index) {
			auto_number->an_state = field_name_is_empty ?
				ANS_AUTO : ANS_MANUAL;
		}

		if (field_name_is_empty)
			*first_idx = (auto_number->an_field_number)++;
	}
	return 1;
}

// 0 on error, 1 on non-error termination, and 2 if it returns a value
static int
FieldNameIterator_next(FieldNameIterator *self, int *is_attribute,
		Py_ssize_t *name_idx, SubString *name) {
	if (self->index >= self->str.end)
		return 1;
	assert(false);
}

static PyObject *
get_field_object(SubString *input, PyObject *args, PyObject *kwargs, AutoNumber *auto_number) {
	PyObject *obj = NULL;
	int ok;
	int is_attribute;
	SubString name;
	SubString first;
	Py_ssize_t index;
	FieldNameIterator rest;

	if (!field_name_split(input->str, input->start, input->end, &first,
			&index, &rest, auto_number)) {
		assert(false);
	}

	if (index == -1) {
		// look up in kwargs
		assert(false);
	} else {
		if (args == NULL) {
			assert(false);
		}
		obj = PySequence_GetItem(args, index);
		if (obj == NULL) {
			assert(false);
		}
	}

	// iterate over the rest of the field_name
	while ((ok = FieldNameIterator_next(&rest, &is_attribute, &index, &name)) == 2) {
		assert(false);
	}

	if (ok == 1)
		return obj;
	assert(false);
}


int _PyUnicode_FormatAdvancedWriter(_PyUnicodeWriter *writer, PyObject *obj, PyObject *format_spec, Py_ssize_t start, Py_ssize_t end);

static int
render_field(PyObject *fieldobj, SubString *format_spec, _PyUnicodeWriter *writer) {
	int (*formatter)(_PyUnicodeWriter *, PyObject *, PyObject *, Py_ssize_t, Py_ssize_t) = NULL;
	int err;

	// printf("format_spec %p %d %d\n", format_spec->str, format_spec->start, format_spec->end);

	// If we know the type exactly, skip the lookup of __format__ and just
	// call the formatter directly
  if (PyUnicode_CheckExact(fieldobj))
    formatter = _PyUnicode_FormatAdvancedWriter;
	else if (PyLong_CheckExact(fieldobj)) {
		formatter = _PyLong_FormatAdvancedWriter;
	} else if (PyFloat_CheckExact(fieldobj)) {
		formatter = _PyFloat_FormatAdvancedWriter;
	} else {
		assert(false);
	}

	if (formatter) { 
		err = formatter(writer, fieldobj, format_spec->str,
			format_spec->start, format_spec->end);
		return (err == 0);
	} else {
		assert(false);
	}
	assert(false);
}

// do the !r or !s conversion on obj
static PyObject *
do_conversion(PyObject *obj, Py_UCS4 conversion) {
  switch (conversion) {
  case 'r':
    return PyObject_Repr(obj);
  default:
    fail(0);
  }
}

static int
output_markup(SubString *field_name, SubString *format_spec,
		int format_spec_needs_expanding, Py_UCS4 conversion,
		_PyUnicodeWriter *writer, PyObject *args, PyObject *kwargs,
		int recursion_depth, AutoNumber *auto_number) {
	PyObject *tmp = NULL;
	PyObject *fieldobj = NULL;
	SubString *actual_format_spec;
	int result = 0;

	// convert field_name to an object
	fieldobj = get_field_object(field_name, args, kwargs, auto_number);
	if (fieldobj == NULL)
		assert(false);
	
	if (conversion != '\0') {
    tmp = do_conversion(fieldobj, conversion);
    if (tmp == NULL || PyUnicode_READY(tmp) == -1)
      goto done;

    Py_DECREF(fieldobj);
    fieldobj = tmp;
    tmp = NULL;
	}

	if (format_spec_needs_expanding) {
		assert(false);
	} else
		actual_format_spec = format_spec;
	
	if (render_field(fieldobj, actual_format_spec, writer) == 0)
		assert(false);
	
	result = 1;

done:
	Py_XDECREF(fieldobj);
	Py_XDECREF(tmp);

	return result;
}

static int
do_markup(SubString *input, PyObject *args, PyObject *kwargs,
		_PyUnicodeWriter *writer, int recursion_depth, AutoNumber *auto_number) {
	MarkupIterator iter;
	int format_spec_needs_expanding;
	int result;
	int field_present;
	SubString literal;
	SubString field_name;
	SubString format_spec;
	Py_UCS4 conversion;

	MarkupIterator_init(&iter, input->str, input->start, input->end);
	while ((result = MarkupIterator_next(
		&iter, &literal, &field_present,
		&field_name, &format_spec,
		&conversion,
		&format_spec_needs_expanding
	)) == 2) {
		if (literal.end != literal.start) {
			if (!field_present && iter.str.start == iter.str.end)
				writer->overallocate = 0;
			if (_PyUnicodeWriter_WriteSubstring(writer, literal.str, literal.start, literal.end) < 0)
				return 0;
		}

		if (field_present) {
			if (iter.str.start == iter.str.end)
				writer->overallocate = 0;
			if (!output_markup(&field_name, &format_spec, format_spec_needs_expanding, conversion, writer, args, kwargs, recursion_depth, auto_number))
				return 0;
		}
	}
	return result;
}

static PyObject *
build_string(SubString *input, PyObject *args, PyObject *kwargs,
		int recursion_depth, AutoNumber *auto_number) {
	_PyUnicodeWriter writer;

	if (recursion_depth <= 0) {
		assert(false);
	}

	_PyUnicodeWriter_Init(&writer);
	writer.overallocate = 1;
	writer.min_length = PyUnicode_GET_LENGTH(input->str) + 100;

	if (!do_markup(input, args, kwargs, &writer, recursion_depth,
			auto_number)) {
		assert(false);
	}
	return _PyUnicodeWriter_Finish(&writer);
}

static PyObject *
do_string_format(PyObject *self, PyObject *args, PyObject *kwargs) {
	#if 0
	printf("do_string_format self is: %s\n", (char *) PyUnicode_DATA(self));
	assert(kwargs == NULL);
	for (int i = 0; i < PyTuple_GET_SIZE(args); ++i) {
		PyObject *obj = PyTuple_GET_ITEM(args, i);
		printf("args %d: %d\n", i, PyLong_AsLong(obj));
	}
	#endif

	SubString input;
	int recursion_depth = 2;

	AutoNumber auto_number;

	if (PyUnicode_READY(self) == -1)
		return NULL;
	
	AutoNumber_Init(&auto_number);
	SubString_init(&input, self, 0, PyUnicode_GET_LENGTH(self));
	return build_string(&input, args, kwargs, recursion_depth, &auto_number);
}
