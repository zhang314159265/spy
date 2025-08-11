#pragma once

#include "internal/pycore_ast.h"

#define _INDENT() fprintf(stderr, "%*s", indent, "")

void dump_expr(expr_ty expr, int indent);
void dump_stmt(stmt_ty stmt, int indent);
void dump_alias(alias_ty alias, int indent);

void dump_stmt_seq(const char *tag, asdl_stmt_seq *stmt_seq, int indent) {
	int l = asdl_seq_LEN(stmt_seq);
	for (int i = 0; i < l; ++i) {
		_INDENT();
		fprintf(stderr, "%sstmt seq %d/%d\n", tag ? tag : "", i + 1, l);
		dump_stmt(asdl_seq_GET_UNTYPED(stmt_seq, i), indent + 2);
	}
}

void dump_withitem_seq(const char *tag, asdl_withitem_seq *seq, int indent) {
	int l = asdl_seq_LEN(seq);
	for (int i = 0; i < l; ++i) {
		_INDENT();
		fprintf(stderr, "%swithitem seq %d/%d\n", tag ? tag : "", i + 1, l);
    withitem_ty item = (withitem_ty) asdl_seq_GET_UNTYPED(seq, i);
    dump_expr(item->context_expr, indent + 2);
    dump_expr(item->optional_vars, indent + 2);
	}
}

void dump_expr_seq(const char *tag, asdl_expr_seq *expr_seq, int indent) {
	int l = asdl_seq_LEN(expr_seq);
	for (int i = 0; i < l; ++i) {
		_INDENT();
		fprintf(stderr, "%sexpr seq %d/%d\n", tag ? tag : "", i + 1, l);
		dump_expr(asdl_seq_GET_UNTYPED(expr_seq, i), indent + 2);
	}
}

void dump_exception_handler(excepthandler_ty ex_handler, int indent);
void dump_excepthandler_seq(const char *tag, asdl_excepthandler_seq *ex_seq, int indent) {
  int l = asdl_seq_LEN(ex_seq);
  for (int i = 0; i < l; ++i) {
    _INDENT();
    fprintf(stderr, "%sex handler seq %d/%d\n", tag ? tag : "", i + 1, l);
    dump_exception_handler(asdl_seq_GET_UNTYPED(ex_seq, i), indent + 2); 
  }
}

void dump_alias_seq(const char *tag, asdl_alias_seq *alias_seq, int indent) {
	int l = asdl_seq_LEN(alias_seq);
	for (int i = 0; i < l; ++i) {
		_INDENT();
		fprintf(stderr, "%salias seq %d/%d\n", tag ? tag : "", i + 1, l);
		dump_alias(asdl_seq_GET_UNTYPED(alias_seq, i), indent + 2);
	}
}

void dump_cmpop_seq(asdl_int_seq *cmpop_seq, int indent) {
	int l = asdl_seq_LEN(cmpop_seq);
	for (int i = 0; i < l; ++i) {
		_INDENT();
		fprintf(stderr, "cmpop seq %d/%d: ", i + 1, l);
		int cmpop = (int) (long) asdl_seq_GET_UNTYPED(cmpop_seq, i);
		switch (cmpop) {
		case Eq:
			fprintf(stderr, "==\n");
			break;
		case Lt:
			fprintf(stderr, "<\n");
			break;
		case Is:
			fprintf(stderr, "is\n");
			break;
		case IsNot:
			fprintf(stderr, "is not\n");
			break;
		case In:
			fprintf(stderr, "in\n");
			break;
		case NotIn:
			fprintf(stderr, "not in\n");
			break;
		default:
			assert(false);
		}
	}
}

void dump_identifier(identifier id, int indent) {
	_INDENT();
  if (!id) {
    fprintf(stderr, "NULL Identifier\n");
    return;  
  }
	assert(PyUnicode_Check(id));
	assert(PyUnicode_KIND(id) == PyUnicode_1BYTE_KIND);
	const char *data = PyUnicode_DATA(id);
	fprintf(stderr, "id[%s]\n", data);
}

void dump_expr_context(expr_context_ty ctx, int indent) {
	_INDENT();
	switch (ctx) {
	case Load: fprintf(stderr, "CTX=LOAD\n"); break;
	case Store: fprintf(stderr, "CTX=STORE\n"); break;
	case Del: fprintf(stderr, "CTX=DEL\n"); break;
	default:
	assert(false);
	}
}

void dump_keyword_seq(const char *tag, asdl_keyword_seq *keyword_seq, int indent) {
  int l = asdl_seq_LEN(keyword_seq);
	for (int i = 0; i < l; ++i) {
		_INDENT();
		fprintf(stderr, "%skeywod seq %d/%d\n", tag ? tag : "", i + 1, l);

    keyword_ty kw = asdl_seq_GET_UNTYPED(keyword_seq, i);
    dump_identifier(kw->arg, indent + 2);
    dump_expr(kw->value, indent + 2);
	}
}

void dump_operator_ty(operator_ty op, int indent) {
	_INDENT();
	switch (op) {
	case Add:
		fprintf(stderr, "+\n"); break;
	case Sub:
		fprintf(stderr, "-\n"); break;
	case Mod:
		fprintf(stderr, "%%\n"); break;
	case Div:
		fprintf(stderr, "/\n"); break;
	case Pow:
		fprintf(stderr, "**\n"); break;
	case Mult:
		fprintf(stderr, "*\n"); break;
	case FloorDiv:
		fprintf(stderr, "//\n"); break;
	case BitAnd:
		fprintf(stderr, "&\n"); break;
	case BitOr:
		fprintf(stderr, "|\n"); break;
	case BitXor:
		fprintf(stderr, "^\n"); break;
	case LShift:
		fprintf(stderr, "<<\n"); break;
	case RShift:
		fprintf(stderr, ">>\n"); break;
	default:
		assert(false);
	}
}

void dump_unaryop_ty(unaryop_ty op, int indent) {
	_INDENT();
	switch (op) {
	case UAdd:
		fprintf(stderr, "+\n"); break;
	case USub:
		fprintf(stderr, "-\n"); break;
	case Invert:
		fprintf(stderr, "~\n"); break;
	case Not:
		fprintf(stderr, "not\n"); break;
	default:
		assert(false);
	}
}

void dump_exception_handler(excepthandler_ty ex_handler, int indent) {
  _INDENT();
  fprintf(stderr, "Except handler\n");

  dump_expr(ex_handler->v.ExceptHandler.type, indent + 2);
  dump_identifier(ex_handler->v.ExceptHandler.name, indent + 2);
  dump_stmt_seq(NULL, ex_handler->v.ExceptHandler.body, indent + 2);
}

const char *boolop_ty_to_str(boolop_ty op) {
  switch (op) {
  case And:
    return "and";
  case Or:
    return "or";
  default:
    fail(0);
  }
}

void dump_expr(expr_ty expr, int indent) {
	_INDENT();
	if (!expr) {
		fprintf(stderr, "NULL\n");
		return;
	}
	switch (expr->kind) {
	case Name_kind:
		fprintf(stderr, "Name:\n");
		dump_identifier(expr->v.Name.id, indent + 2);
		dump_expr_context(expr->v.Name.ctx, indent + 2);
		break;
	case Call_kind:
		fprintf(stderr, "Call:\n");	
		dump_expr(expr->v.Call.func, indent + 2);
		dump_expr_seq(NULL, expr->v.Call.args, indent + 2);
		dump_keyword_seq(NULL, expr->v.Call.keywords, indent + 2);
		break;
	case Constant_kind: {
		PyObject *o = expr->v.Constant.value;
		fprintf(stderr, "Constant: ");
		// only support string constant now
		if (PyUnicode_CheckExact(expr->v.Constant.value)) {
			fprintf(stderr, "'%s'\n", (char *) PyUnicode_DATA(expr->v.Constant.value));
		} else if (PyLong_Check(expr->v.Constant.value)) {
			fprintf(stderr, "%ld\n", PyLong_AsLong(expr->v.Constant.value));
		} else if (Py_IsTrue(expr->v.Constant.value)) {
			fprintf(stderr, "True\n");
		} else if (PyFloat_CheckExact(expr->v.Constant.value)) {
			fprintf(stderr, "%lf\n", ((PyFloatObject *) o)->ob_fval);
		} else {
			printf("Unhandled constant type %s\n", Py_TYPE(expr->v.Constant.value)->tp_name);
			assert(false);
		}
		break;
	}
	case BinOp_kind:
		fprintf(stderr, "BinOp:\n");
		dump_expr(expr->v.BinOp.left, indent + 2);
		dump_operator_ty(expr->v.BinOp.op, indent + 2);
		dump_expr(expr->v.BinOp.right, indent + 2);
		break;
	case Attribute_kind:
		fprintf(stderr, "Attribute %s:\n", (char *) PyUnicode_DATA(expr->v.Attribute.attr));
		dump_expr(expr->v.Attribute.value, indent + 2);
		dump_expr_context(expr->v.Attribute.ctx, indent + 2);
		break;
	case Compare_kind:
		fprintf(stderr, "Compare:\n");
		dump_expr(expr->v.Compare.left, indent + 2);
		dump_cmpop_seq(expr->v.Compare.ops, indent + 2);
		dump_expr_seq(NULL, expr->v.Compare.comparators, indent + 2);
		break;
	case Tuple_kind:
		fprintf(stderr, "Tuple:\n");
		dump_expr_seq(NULL, expr->v.Tuple.elts, indent + 2);
		dump_expr_context(expr->v.Tuple.ctx, indent + 2);
		break;
	case List_kind:
		fprintf(stderr, "List:\n");
		dump_expr_seq(NULL, expr->v.List.elts, indent + 2);
		dump_expr_context(expr->v.List.ctx, indent + 2);
		break;
	case Starred_kind:
		fprintf(stderr, "Starred:\n");
		dump_expr(expr->v.Starred.value, indent + 2);
		dump_expr_context(expr->v.Starred.ctx, indent + 2);
		break;
	case Dict_kind:
		fprintf(stderr, "Dict:\n");
		dump_expr_seq(NULL, expr->v.Dict.keys, indent + 2);
		dump_expr_seq(NULL, expr->v.Dict.values, indent + 2);
		break;
	case Subscript_kind:
		fprintf(stderr, "Subscript:\n");
		dump_expr(expr->v.Subscript.value, indent + 2);
		dump_expr(expr->v.Subscript.slice, indent + 2);
		dump_expr_context(expr->v.Subscript.ctx, indent + 2);
		break;
	case Set_kind:
		fprintf(stderr, "Set:\n");
		dump_expr_seq(NULL, expr->v.Set.elts, indent + 2);
		break;
	case UnaryOp_kind:
		fprintf(stderr, "UnaryOp:\n");
		dump_unaryop_ty(expr->v.UnaryOp.op, indent + 2);
		dump_expr(expr->v.UnaryOp.operand, indent + 2);
		break;
	case Slice_kind:
		fprintf(stderr, "Slice:\n");
		dump_expr(expr->v.Slice.lower, indent + 2);
		dump_expr(expr->v.Slice.upper, indent + 2);
		dump_expr(expr->v.Slice.step, indent + 2);
		break;
  case Yield_kind:
    fprintf(stderr, "Yield:\n");
    dump_expr(expr->v.Yield.value, indent + 2);
    break;
  case IfExp_kind:
    fprintf(stderr, "IfExp:\n");
    dump_expr(expr->v.IfExp.test, indent + 2);
    dump_expr(expr->v.IfExp.body, indent + 2);
    dump_expr(expr->v.IfExp.orelse, indent + 2);
    break;
  case BoolOp_kind:
    fprintf(stderr, "BoolOp: %s\n", boolop_ty_to_str(expr->v.BoolOp.op));
    dump_expr_seq(NULL, expr->v.BoolOp.values, indent + 2);
    break;
	default:
		printf("expr->kind is %d\n", expr->kind);
		assert(false && "dump_expr");
	}
}

void dump_arguments_ty(arguments_ty args, int indent) {
  int l;

  l = asdl_seq_LEN(args->posonlyargs);
	for (int i = 0; i < l; ++i) {
		_INDENT();
		fprintf(stderr, "posonly param seq %d/%d: ", i + 1, l);
		arg_ty arg = (arg_ty) asdl_seq_GET_UNTYPED(args->posonlyargs, i);
		fprintf(stderr, "%s\n", (char *) PyUnicode_DATA(arg->arg));
	}

  l = asdl_seq_LEN(args->args);
	for (int i = 0; i < l; ++i) {
		_INDENT();
		fprintf(stderr, "param seq %d/%d: ", i + 1, l);
		arg_ty arg = (arg_ty) asdl_seq_GET_UNTYPED(args->args, i);
		fprintf(stderr, "%s\n", (char *) PyUnicode_DATA(arg->arg));
	}

  l = asdl_seq_LEN(args->kwonlyargs);
	for (int i = 0; i < l; ++i) {
		_INDENT();
		fprintf(stderr, "kwonly param seq %d/%d: ", i + 1, l);
		arg_ty arg = (arg_ty) asdl_seq_GET_UNTYPED(args->kwonlyargs, i);
		fprintf(stderr, "%s\n", (char *) PyUnicode_DATA(arg->arg));
	}

  if (args->vararg) {
    _INDENT();
    fprintf(stderr, "vararg %s\n", (char *) PyUnicode_DATA(args->vararg->arg));
  }
  if (args->kwarg) {
    _INDENT();
    fprintf(stderr, "kwarg %s\n", (char *) PyUnicode_DATA(args->kwarg->arg));
  }
}

void dump_alias(alias_ty alias, int indent) {
  _INDENT();
  fprintf(stderr, "%s as %s\n", (char*) PyUnicode_DATA(alias->name), alias->asname ? (char *) PyUnicode_DATA(alias->asname) : "ITSELF");
}


void dump_stmt(stmt_ty stmt, int indent) {
	_INDENT();
	switch (stmt->kind) {
	case Expr_kind:
		fprintf(stderr, "ExprStmt:\n");
		dump_expr(stmt->v.Expr.value, indent + 2);
		break;
	case FunctionDef_kind:
		fprintf(stderr, "FunctionDef: %s\n", (char *) PyUnicode_DATA(stmt->v.FunctionDef.name));
		dump_arguments_ty(stmt->v.FunctionDef.args, indent + 2);
		dump_stmt_seq(NULL, stmt->v.FunctionDef.body, indent + 2);
    dump_expr_seq("Decorators ", stmt->v.FunctionDef.decorator_list, indent + 2);
		break;
	case Assign_kind:
		fprintf(stderr, "Assign:\n");
		dump_expr_seq(NULL, stmt->v.Assign.targets, indent + 2);
		dump_expr(stmt->v.Assign.value, indent + 2);
		break;
	case AugAssign_kind:
		fprintf(stderr, "AugAssign:\n");
		dump_expr(stmt->v.AugAssign.target, indent + 2);
		dump_operator_ty(stmt->v.AugAssign.op, indent + 2);
		dump_expr(stmt->v.AugAssign.value, indent + 2);
		break;
	case Return_kind:
		fprintf(stderr, "Return:\n");
		dump_expr(stmt->v.Return.value, indent + 2);
		break;
	case For_kind:
		fprintf(stderr, "For:\n");
		dump_expr(stmt->v.For.target, indent + 2);
		dump_expr(stmt->v.For.iter, indent + 2);
		dump_stmt_seq(NULL, stmt->v.For.body, indent + 2);
		assert(!stmt->v.For.orelse);
		break;
	case If_kind:
		fprintf(stderr, "If:\n");
		dump_expr(stmt->v.If.test, indent + 2);
		dump_stmt_seq(NULL, stmt->v.If.body, indent + 2);
		dump_stmt_seq(NULL, stmt->v.If.orelse, indent + 2);
		break;
	case While_kind:
		fprintf(stderr, "While:\n");
		dump_expr(stmt->v.While.test, indent + 2);
		dump_stmt_seq(NULL, stmt->v.While.body, indent + 2);
		dump_stmt_seq(NULL, stmt->v.While.orelse, indent + 2);
		break;
	case Break_kind:
		fprintf(stderr, "Break\n");
		break;
	case Delete_kind:
		fprintf(stderr, "Del\n");
		dump_expr_seq(NULL, stmt->v.Delete.targets, indent + 2);
		break;
	case ClassDef_kind:
		fprintf(stderr, "ClassDef: %s\n", (char *) PyUnicode_DATA(stmt->v.ClassDef.name));
    dump_expr_seq(NULL, stmt->v.ClassDef.bases, indent + 2);
		assert(!stmt->v.ClassDef.keywords);
		dump_stmt_seq(NULL, stmt->v.ClassDef.body, indent + 2);
		dump_expr_seq(NULL, stmt->v.ClassDef.decorator_list, indent + 2);
		break;	
  case Pass_kind:
    fprintf(stderr, "Pass\n");
    break;
  case With_kind:
    fprintf(stderr, "With\n");
    dump_withitem_seq(NULL, stmt->v.With.items, indent + 2);
    dump_stmt_seq(NULL, stmt->v.With.body, indent + 2);
    break;
  case ImportFrom_kind:
    fprintf(stderr, "ImportFrom %s %d\n", (char *) PyUnicode_DATA(stmt->v.ImportFrom.module), stmt->v.ImportFrom.level);
    dump_alias_seq(NULL, stmt->v.ImportFrom.names, indent + 2);
    break;
  case Try_kind:
    fprintf(stderr, "Try\n");
    dump_stmt_seq(NULL, stmt->v.Try.body, indent + 2);
    dump_excepthandler_seq(NULL, stmt->v.Try.handlers, indent + 2);
    dump_stmt_seq(NULL, stmt->v.Try.orelse, indent + 2);
    dump_stmt_seq(NULL, stmt->v.Try.finalbody, indent + 2);
    break;
  case Raise_kind:
    fprintf(stderr, "Raise\n");
    dump_expr(stmt->v.Raise.exc, indent + 2);
    dump_expr(stmt->v.Raise.cause, indent + 2);
    break;
  case Assert_kind:
    fprintf(stderr, "Assert\n");
    dump_expr(stmt->v.Assert.test, indent + 2);
    dump_expr(stmt->v.Assert.msg, indent + 2);
    break;
	default:
		fprintf(stderr, "Can not dump statement of type %d\n", stmt->kind);
		assert(false && "dump_stmt");
	}
}

void dump_mod(mod_ty mod, int indent) {
	assert(mod->kind == Module_kind);
	_INDENT();
	fprintf(stderr, "module:\n");
	for (int i = 0; i < asdl_seq_LEN(mod->v.Module.body); ++i) {
		dump_stmt(asdl_seq_GET_UNTYPED(mod->v.Module.body, i), indent + 2);
	}
	fprintf(stderr, "dump done\n");
}
