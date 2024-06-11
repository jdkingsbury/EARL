// MIT License

// Copyright (c) 2023 malloc-nbytes

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <cassert>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <memory>

#include "utils.hpp"
#include "interpreter.hpp"
#include "intrinsics.hpp"
#include "err.hpp"
#include "token.hpp"
#include "ast.hpp"
#include "ctx.hpp"
#include "common.hpp"
#include "earl.hpp"

earl::runtime::evalres::Obj eval_stmt(Stmt *stmt, Ctx &ctx);
earl::runtime::evalres::Obj eval_stmt_block(StmtBlock *block, Ctx &ctx);

earl::runtime::evalres::Obj eval_expr_funccall(ExprFuncCall *expr, Ctx &ctx) {
    UNIMPLEMENTED("eval_expr_funccall");
}

earl::runtime::evalres::Obj eval_expr_term(ExprTerm *expr, Ctx &ctx) {
    UNIMPLEMENTED("eval_expr_term");
}

earl::runtime::evalres::Obj eval_expr_bin(ExprBinary *expr, Ctx &ctx) {
    UNIMPLEMENTED("eval_expr_bin");
}

earl::runtime::evalres::Obj Interpreter::eval_expr(Expr *expr, Ctx &ctx) {
    UNIMPLEMENTED("eval_expr");
}

earl::runtime::evalres::Obj eval_stmt_let(StmtLet *stmt, Ctx &ctx) {
    const std::string &id = stmt->m_id->lexeme();

    if (ctx.variable_is_registered(id)) {
        Err::err_wtok(stmt->m_id.get());
        ERR_WARGS(Err::Type::Redeclared, "variable `%s` is already defined", stmt->m_id->lexeme().c_str());
    }

    std::vector<earl::primitive::Type> binding_type = earl::primitive::of_str(stmt->m_type.get()->lexeme());

    earl::runtime::evalres::Obj expr_eval = Interpreter::eval_expr(stmt->m_expr.get(), ctx);

    return earl::runtime::evalres::Obj(earl::runtime::value::Unit(nullptr));
}

earl::runtime::evalres::Obj eval_stmt_expr(StmtExpr *stmt, Ctx &ctx) {
    UNIMPLEMENTED("eval_stmt_expr");
}

earl::runtime::evalres::Obj eval_stmt_block(StmtBlock *block, Ctx &ctx) {
    UNIMPLEMENTED("eval_stmt_block");
}

// When we hit a statement `def` (a function declaration),
// we do not actually want to execute this function.
// We just want to add it to the global context so it
// can be called later from either a statement expression
// or a right-hand-side assignment.
earl::runtime::evalres::Obj eval_stmt_def(StmtDef *stmt, Ctx &ctx) {
    UNIMPLEMENTED("eval_stmt_def");
}

earl::runtime::evalres::Obj eval_stmt_if(StmtIf *stmt, Ctx &ctx) {
    UNIMPLEMENTED("eval_stmt_if");
}

earl::runtime::evalres::Obj eval_stmt_return(StmtReturn *stmt, Ctx &ctx) {
    UNIMPLEMENTED("eval_stmt_return");
}

earl::runtime::evalres::Obj eval_stmt_mut(StmtMut *stmt, Ctx &ctx) {
    UNIMPLEMENTED("eval_stmt_mut");
}

earl::runtime::evalres::Obj eval_stmt_while(StmtWhile *stmt, Ctx &ctx) {
    UNIMPLEMENTED("eval_stmt_while");
}

earl::runtime::evalres::Obj eval_stmt_for(StmtFor *stmt, Ctx &ctx) {
    UNIMPLEMENTED("eval_stmt_for");
}

earl::runtime::evalres::Obj eval_stmt(Stmt *stmt, Ctx &ctx) {
    switch (stmt->stmt_type()) {
    case StmtType::Let: {
        return eval_stmt_let(dynamic_cast<StmtLet *>(stmt), ctx);
    } break;
    case StmtType::Mut: {
        return eval_stmt_mut(dynamic_cast<StmtMut *>(stmt), ctx);
    } break;
    case StmtType::Def: {
        return eval_stmt_def(dynamic_cast<StmtDef *>(stmt), ctx);
    } break;
    case StmtType::Block: {
        assert(false && "unimplemented");
    } break;
    case StmtType::Stmt_Expr: {
        return eval_stmt_expr(dynamic_cast<StmtExpr *>(stmt), ctx);
    } break;
    case StmtType::If: {
        return eval_stmt_if(dynamic_cast<StmtIf *>(stmt), ctx);
    } break;
    case StmtType::Stmt_Return: {
        return eval_stmt_return(dynamic_cast<StmtReturn *>(stmt), ctx);
    } break;
    case StmtType::Stmt_While: {
        return eval_stmt_while(dynamic_cast<StmtWhile *>(stmt), ctx);
    } break;
    case StmtType::Stmt_For: {
        return eval_stmt_for(dynamic_cast<StmtFor *>(stmt), ctx);
    } break;
    default:
        assert(false && "eval_stmt: invalid statement");
    }
}

earl::runtime::evalres::Obj Interpreter::interpret(Program &program) {
    Ctx ctx;

    for (size_t i = 0; i < program.m_stmts.size(); ++i) {
        eval_stmt(program.m_stmts.at(i).get(), ctx);
    }

    return earl::runtime::evalres::Obj{0};
}
