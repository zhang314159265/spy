// This file should be automatically generated

#pragma once

int PyAST_Check(PyObject *obj) {
  struct ast_state *state = get_ast_state();
  if (state == NULL) {
    return -1;
  }
  return PyObject_IsInstance(obj, state->AST_type);
}

typedef struct {
  PyObject_HEAD
  PyObject *dict;
} AST_object;


static PyType_Slot AST_type_slots[] = {
  {0, 0},
};

static PyType_Spec AST_type_spec = {
  "ast.AST",
  sizeof(AST_object),
  0,
  Py_TPFLAGS_BASETYPE,
  AST_type_slots,
};

static int init_types(struct ast_state *state) {
  assert(state->initialized >= 0);

  if (state->initialized) {
    return 1;
  }

  state->AST_type = PyType_FromSpec(&AST_type_spec);
  if (!state->AST_type) {
    return 0;
  }

  state->initialized = 1;
  return 1;
}

static struct ast_state *get_ast_state(void) {
  PyInterpreterState *interp = _PyInterpreterState_GET();
  struct ast_state *state = &interp->ast;
  if (!init_types(state)) {
    return NULL;
  }
  return state;
}

