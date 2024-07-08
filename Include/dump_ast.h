#pragma once

#include "internal/pycore_ast.h"

#define INDENT() fprintf(stderr, "%*s", indent, "")

void dump_expr(expr_ty expr, int indent);

void dump_expr_seq(asdl_expr_seq *expr_seq, int indent) {
	int l = asdl_seq_LEN(expr_seq);
	for (int i = 0; i < l; ++i) {
		INDENT();
		fprintf(stderr, "expr seq %d/%d\n", i + 1, l);
		dump_expr(asdl_seq_GET_UNTYPED(expr_seq, i), indent + 2);
	}
}

void dump_identifier(identifier id, int indent) {
	assert(PyUnicode_Check(id));
	assert(PyUnicode_KIND(id) == PyUnicode_1BYTE_KIND);
	const char *data = PyUnicode_DATA(id);
	INDENT();
	fprintf(stderr, "id[%s]\n", data);
}

void dump_expr_context(expr_context_ty ctx, int indent) {
	INDENT();
	switch (ctx) {
	case Load: fprintf(stderr, "CTX=LOAD\n"); break;
	case Store: fprintf(stderr, "CTX=STORE\n"); break;
	case Del: fprintf(stderr, "CTX=DEL\n"); break;
	default:
	assert(false);
	}
}

void dump_keyword_seq(asdl_keyword_seq *keyword_seq, int indent) {
	assert(!keyword_seq);
}

void dump_expr(expr_ty expr, int indent) {
	INDENT();
	switch (expr->kind) {
	case Name_kind:
		fprintf(stderr, "Name:\n");
		dump_identifier(expr->v.Name.id, indent + 2);
		dump_expr_context(expr->v.Name.ctx, indent + 2);
		break;
	case Call_kind:
		fprintf(stderr, "Call:\n");	
		dump_expr(expr->v.Call.func, indent + 2);
		dump_expr_seq(expr->v.Call.args, indent + 2);
		dump_keyword_seq(expr->v.Call.keywords, indent + 2);
		break;
	case Constant_kind:
		fprintf(stderr, "Constant: ");
		// only support string constant now
		assert(PyUnicode_CheckExact(expr->v.Constant.value));
		fprintf(stderr, "'%s'\n", (char *) PyUnicode_DATA(expr->v.Constant.value));
		break;
	default:
		printf("expr->kind is %d\n", expr->kind);
		assert(false && "dump_expr");
	}
}

void dump_stmt(stmt_ty stmt, int indent) {
	INDENT();
	switch (stmt->kind) {
	case Expr_kind:
		fprintf(stderr, "ExprStmt:\n");
		dump_expr(stmt->v.Expr.value, indent + 2);
		break;
	default:
		assert(false && "dump_stmt");
	}
}

void dump_mod(mod_ty mod, int indent) {
	assert(mod->kind == Module_kind);
	INDENT();
	fprintf(stderr, "module:\n");
	for (int i = 0; i < asdl_seq_LEN(mod->v.Module.body); ++i) {
		dump_stmt(asdl_seq_GET_UNTYPED(mod->v.Module.body, i), indent + 2);
	}
}
