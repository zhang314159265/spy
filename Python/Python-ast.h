// Corresponding to cpy/Python/Python-ast.c
// Should be automatically generated by Parser/asdl.c.py

arg_ty
_PyAST_arg(identifier arg, expr_ty annotation, string type_comment) {
	arg_ty p;
	if (!arg) {
		assert(false);
	}
	p = (arg_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->arg = arg;
	assert(annotation == NULL);
	assert(type_comment == NULL);
	return p;
}

arguments_ty
_PyAST_arguments(asdl_arg_seq *args) {
	arguments_ty p;
	p = (arguments_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->args = args;
	return p;
}

stmt_ty
_PyAST_Assign(asdl_expr_seq *targets, expr_ty value, string type_comment) {
	stmt_ty p;
	if (!value) {
		assert(false);
	}
	p = (stmt_ty) malloc(sizeof(*p));
	if (!p) {
		return NULL;
	}
	p->kind = Assign_kind;
	p->v.Assign.targets = targets;
	p->v.Assign.value = value;
	return p;
}

stmt_ty
_PyAST_FunctionDef(identifier name, arguments_ty args, asdl_stmt_seq *body,
		asdl_expr_seq *decorator_list, expr_ty  returns) {
	stmt_ty p;
	assert(!decorator_list);
	assert(!returns);

	if (!name) {
		assert(false);
	}
	if (!args) {
		assert(false);
	}

	p = (stmt_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->kind = FunctionDef_kind;
	p->v.FunctionDef.name = name;
	p->v.FunctionDef.args = args;
	p->v.FunctionDef.body = body;
	p->v.FunctionDef.decorator_list = decorator_list;
	p->v.FunctionDef.returns = returns;
	p->v.FunctionDef.type_comment = NULL;

	return p;
}

stmt_ty
_PyAST_Return(expr_ty value) {
	stmt_ty p;
	p = (stmt_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->kind = Return_kind;
	p->v.Return.value = value;
	return p;
}

stmt_ty
_PyAST_AugAssign(expr_ty target, operator_ty op, expr_ty value) {
	stmt_ty p;
	if (!target) {
		assert(false);
	}
	if (!op) {
		assert(false);
	}
	if (!value) {
		assert(false);
	}
	p = (stmt_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->kind = AugAssign_kind;
	p->v.AugAssign.target = target;
	p->v.AugAssign.op = op;
	p->v.AugAssign.value = value;
	return p;
}

expr_ty
_PyAST_BinOp(expr_ty left, operator_ty op, expr_ty right) {
	expr_ty p;
	if (!left) {
		assert(false);
	}
	if (!op) {
		assert(false);
	}
	if (!right) {
		assert(false);
	}
	p = (expr_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->kind = BinOp_kind;
	p->v.BinOp.left = left;
	p->v.BinOp.op = op;
	p->v.BinOp.right = right;
	return p;
}

stmt_ty
_PyAST_For(expr_ty target, expr_ty iter, asdl_stmt_seq *body, asdl_stmt_seq *orelse) {
	stmt_ty p;
	if (!target) {
		assert(false);
	}
	if (!iter) {
		assert(false);
	}
	p = (stmt_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->kind = For_kind;
	p->v.For.target = target;
	p->v.For.iter = iter;
	p->v.For.body = body;
	p->v.For.orelse = orelse;
	return p;
}

expr_ty
_PyAST_Attribute(expr_ty value, identifier attr, expr_context_ty ctx) {
	expr_ty p;
	if (!value) {
		assert(false);
	}
	if (!attr) {
		assert(false);
	}
	if (!ctx) {
		assert(false);
	}
	p = (expr_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->kind = Attribute_kind;
	p->v.Attribute.value = value;
	p->v.Attribute.attr = attr;
	p->v.Attribute.ctx = ctx;
	return p;
}

expr_ty
_PyAST_Compare(expr_ty left, asdl_int_seq *ops, asdl_expr_seq *comparators) {
	expr_ty p;
	if (!left) {
		assert(false);
	}
	p = (expr_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->kind = Compare_kind;
	p->v.Compare.left = left;
	p->v.Compare.ops = ops;
	p->v.Compare.comparators = comparators;
	return p;
}

stmt_ty
_PyAST_If(expr_ty test, asdl_stmt_seq *body, asdl_stmt_seq *orelse) {
	stmt_ty p;
	if (!test) {
		assert(false);
	}
	p = (stmt_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->kind = If_kind;
	p->v.If.test = test;
	p->v.If.body = body;
	p->v.If.orelse = orelse;
	return p;
}

stmt_ty
_PyAST_Break() {
	stmt_ty p;
	p = (stmt_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->kind = Break_kind;
	return p;
}

expr_ty
_PyAST_Tuple(asdl_expr_seq *elts, expr_context_ty ctx) {
	expr_ty p;
	if (!ctx) {
		assert(false);
	}
	p = (expr_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->kind = Tuple_kind;
	p->v.Tuple.elts = elts;
	// printf("tuple elts is %p\n", elts); assert(false);
	p->v.Tuple.ctx = ctx;
	return p;
}

expr_ty
_PyAST_List(asdl_expr_seq *elts, expr_context_ty ctx) {
	expr_ty p;
	if (!ctx) {
		assert(false);
	}
	p = (expr_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->kind = List_kind;
	p->v.List.elts = elts;
	p->v.List.ctx = ctx;
	return p;
}

expr_ty
_PyAST_Starred(expr_ty value, expr_context_ty ctx) {
	expr_ty p;
	if (!value) {
		assert(false);
	}
	if (!ctx) {
		assert(false);
	}
	p = (expr_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->kind = Starred_kind;
	p->v.Starred.value = value;
	p->v.Starred.ctx = ctx;
	return p;
}

expr_ty
_PyAST_Dict(asdl_expr_seq *keys, asdl_expr_seq *values) {
	expr_ty p;
	p = (expr_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->kind = Dict_kind;
	p->v.Dict.keys = keys;
	p->v.Dict.values = values;
	return p;
}

expr_ty
_PyAST_Subscript(expr_ty value, expr_ty slice, expr_context_ty ctx) {
	expr_ty p;
	if (!value) {
		assert(false);
	}
	if (!slice) {
		assert(false);
	}
	if (!ctx) {
		assert(false);
	}
	p = (expr_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->kind = Subscript_kind;
	p->v.Subscript.value = value;
	p->v.Subscript.slice = slice;
	p->v.Subscript.ctx = ctx;
	return p;
}

expr_ty
_PyAST_Set(asdl_expr_seq *elts) {
	expr_ty p;
	p = (expr_ty) malloc(sizeof(*p));
	if (!p)
		return NULL;
	p->kind = Set_kind;
	p->v.Set.elts = elts;
	return p;
}
