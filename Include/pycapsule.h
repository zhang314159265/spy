#pragma once

typedef void (*PyCapsule_Destructor)(PyObject *);

/* Internal structore of PyCapsule */
typedef struct {
	PyObject_HEAD
	void *pointer;
	const char *name;
	void *context;
	PyCapsule_Destructor destructor;
} PyCapsule;

static void capsule_dealloc(PyObject *o);

PyTypeObject PyCapsule_Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	.tp_name = "PyCapsule",
	.tp_basicsize = sizeof(PyCapsule),
	.tp_dealloc = capsule_dealloc,
};

PyObject *PyCapsule_New(void *pointer, const char *name, PyCapsule_Destructor destructor) {
	PyCapsule *capsule;

	if (!pointer) {
		assert(false);
	}

	capsule = PyObject_New(PyCapsule, &PyCapsule_Type);
	if (capsule == NULL) {
		return NULL;
	}
	
	capsule->pointer = pointer;
	capsule->name = name;
	capsule->context = NULL;
	capsule->destructor = destructor;

	return (PyObject *) capsule;
}

static int
name_matches(const char *name1, const char *name2) {
	if (!name1 || !name2) {
		return name1 == name2;
	}
	return !strcmp(name1, name2);
}

void *
PyCapsule_GetPointer(PyObject *o, const char *name) {
	PyCapsule *capsule = (PyCapsule *) o;

	if (!name_matches(name, capsule->name)) {
		assert(false);
	}

	return capsule->pointer;
}

static void capsule_dealloc(PyObject *o) {
	PyCapsule *capsule = (PyCapsule *) o;
	if (capsule->destructor) {
		capsule->destructor(o);
	}
	PyObject_Free(o);
}
