/** @file */

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

#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <memory>

#include "parser.hpp"
#include "utils.hpp"
#include "interpreter.hpp"
#include "intrinsics.hpp"
#include "err.hpp"
#include "token.hpp"
#include "ast.hpp"
#include "ctx.hpp"
#include "common.hpp"
#include "earl.hpp"
#include "lexer.hpp"

earl::value::Obj *eval_stmt(Stmt *stmt, Ctx &ctx);
earl::value::Obj *eval_stmt_block(StmtBlock *block, Ctx &ctx);

earl::value::Obj *eval_user_defined_function(earl::function::Obj *func, std::vector<earl::value::Obj *> &params, Ctx &ctx) {
    ctx.set_function(func);
    func->load_parameters(params);

    earl::value::Obj *result = eval_stmt_block(func->block(), ctx);

    func->clear_locals();

    ctx.unset_function();
    return result;
}

earl::value::Obj *eval_user_defined_class_method(earl::function::Obj *method, std::vector<earl::value::Obj *> &params, earl::value::Class *klass, Ctx &ctx) {
    if (!method->is_pub()) {
        ERR_WARGS(Err::Type::Fatal, "method `%s` in class `%s` does not contain the @pub attribute",
                  method->id().c_str(), klass->id().c_str());
    }

    ctx.set_function(method);
    ctx.curclass = klass;

    method->load_parameters(params);

    for (size_t i = 0; i < klass->m_members.size(); ++i) {
        ctx.register_variable(klass->m_members[i].get());
    }

    earl::value::Obj *result = eval_stmt_block(method->block(), ctx);

    method->clear_locals();
    ctx.unset_function();
    ctx.curclass = nullptr;
    return result;
}

earl::value::Obj *eval_expr_module_funccall(ExprFuncCall *expr, Ctx &main_ctx, Ctx &mod_ctx) {
    std::vector<earl::value::Obj *> params;
    for (size_t i = 0; i < expr->m_params.size(); ++i) {
        params.push_back(Interpreter::eval_expr(expr->m_params.at(i).get(), main_ctx));
    }

    earl::function::Obj *func = mod_ctx.get_registered_function(expr->m_id->lexeme());
    if (!func->is_pub()) {
        ERR_WARGS(Err::Type::Fatal, "function %s does not contain the @pub attribute",
                  func->id().c_str());
    }

    return eval_user_defined_function(func, params, mod_ctx);
}

earl::value::Obj *eval_stmt_let(StmtLet *stmt, Ctx &ctx) {
    if (ctx.variable_is_registered(stmt->m_id->lexeme())) {
        ERR_WARGS(Err::Type::Redeclared,
                  "variable `%s` is already declared", stmt->m_id->lexeme().c_str());
    }

    earl::value::Obj *rhs_result = Interpreter::eval_expr(stmt->m_expr.get(), ctx);

    earl::variable::Obj *created_variable = nullptr;

    if (stmt->m_expr->get_type() == ExprType::Term
        && (dynamic_cast<ExprTerm *>(stmt->m_expr.get())->get_term_type() != ExprTermType::Ident)) {
        created_variable = new earl::variable::Obj(stmt->m_id.get(), std::unique_ptr<earl::value::Obj>(rhs_result), stmt->m_attrs);
        goto reg;
    }

    if ((stmt->m_attrs & static_cast<uint32_t>(Attr::Ref)) != 0) {
        created_variable
            = new earl::variable::Obj(stmt->m_id.get(), std::unique_ptr<earl::value::Obj>(rhs_result), stmt->m_attrs);
    }
    else {
        created_variable
            = new earl::variable::Obj(stmt->m_id.get(), std::unique_ptr<earl::value::Obj>(rhs_result->copy()), stmt->m_attrs);
    }

 reg:
    ctx.register_variable(created_variable);

    return new earl::value::Void();
}

void load_class_members(StmtLet *stmt, earl::value::Class *klass, Ctx &ctx) {
    if (ctx.variable_is_registered(stmt->m_id->lexeme())) {
        ERR_WARGS(Err::Type::Redeclared,
                  "variable `%s` is already declared", stmt->m_id->lexeme().c_str());
    }

    earl::value::Obj *rhs_result = Interpreter::eval_expr(stmt->m_expr.get(), ctx);

    earl::variable::Obj *created_variable = nullptr;

    if (stmt->m_expr->get_type() == ExprType::Term
        && (dynamic_cast<ExprTerm *>(stmt->m_expr.get())->get_term_type() != ExprTermType::Ident)) {
        created_variable = new earl::variable::Obj(stmt->m_id.get(), std::unique_ptr<earl::value::Obj>(rhs_result), stmt->m_attrs);
        goto reg;
    }

    if ((stmt->m_attrs & static_cast<uint32_t>(Attr::Ref)) != 0) {
        created_variable
            = new earl::variable::Obj(stmt->m_id.get(), std::unique_ptr<earl::value::Obj>(rhs_result), stmt->m_attrs);
    }
    else {
        created_variable
            = new earl::variable::Obj(stmt->m_id.get(), std::unique_ptr<earl::value::Obj>(rhs_result->copy()), stmt->m_attrs);
    }

 reg:
    // ctx.register_variable(created_variable);
    klass->add_member(std::unique_ptr<earl::variable::Obj>(created_variable));
}

earl::value::Obj *eval_class_instantiation(ExprFuncCall *expr, Ctx &ctx) {
    // Make sure the class exists
    StmtClass *stmt = nullptr;
    for (size_t i = 0; i < ctx.available_classes.size(); ++i) {
        if (ctx.available_classes[i]->m_id->lexeme() == expr->m_id->lexeme()) {
            stmt = ctx.available_classes[i];
        }
    }

    if (!stmt) {
        ERR_WARGS(Err::Type::Fatal, "class `%s` does not exist", expr->m_id->lexeme().c_str());
    }

    // Create new instance of the class
    earl::value::Class *klass = new earl::value::Class(stmt);

    // Go through the constructor args and add the available variables
    for (size_t i = 0; i < stmt->m_constructor_args.size(); ++i) {
        klass->add_member_assignee(stmt->m_constructor_args[i].get());
    }

    // Add the class methods
    for (size_t i = 0; i < stmt->m_methods.size(); ++i) {
        klass->add_method(std::make_unique<earl::function::Obj>(stmt->m_methods[i].get(),
                                                                &stmt->m_methods[i]->m_args));
    }

    std::vector<Token *> &available_idents = klass->m_member_assignees;

    // Evaluate [x, y, z,...] in class def and add each one
    // to a temporary scope.
    for (size_t i = 0; i < expr->m_params.size(); ++i) {
        auto *value = Interpreter::eval_expr(expr->m_params[i].get(), ctx);
        auto *var = new earl::variable::Obj(available_idents[i],
                                            std::unique_ptr<earl::value::Obj>(value));

        // We do not want these variables to be always accessible.
        // So add them to a temporary scope
        ctx.add_to_tmp_scope(var);
    }

    // Evaluate all of the members of the class by either using
    // their default specified values or by using what was supplied
    // to the constructor during class instantiation.
    for (size_t i = 0; i < klass->m_stmtclass->m_members.size(); ++i) {
        StmtLet *let = klass->m_stmtclass->m_members[i].get();
        load_class_members(let, klass, ctx);
    }

    // Clear the constructor variables
    ctx.clear_tmp_scope();

    return klass;
}

earl::value::Obj *eval_expr_funccall(ExprFuncCall *expr, Ctx &ctx) {
    std::vector<earl::value::Obj *> params;
    for (size_t i = 0; i < expr->m_params.size(); ++i) {
        params.push_back(Interpreter::eval_expr(expr->m_params.at(i).get(), ctx));
    }

    // Check if we are creating a new class instance
    if (ctx.class_is_registered(expr->m_id->lexeme())) {
        return eval_class_instantiation(expr, ctx);
    }

    // Check if the funccall is intrinsic
    if (Intrinsics::is_intrinsic(expr->m_id->lexeme())) {
        return Intrinsics::call(expr, params, ctx);
    }

    earl::function::Obj *func = ctx.get_registered_function(expr->m_id->lexeme());

    return eval_user_defined_function(func, params, ctx);
}

earl::value::Obj *eval_expr_list_literal(ExprListLit *expr, Ctx &ctx) {
    std::vector<earl::value::Obj *> list;
    for (size_t i = 0; i < expr->m_elems.size(); ++i) {
        list.push_back(Interpreter::eval_expr(expr->m_elems.at(i).get(), ctx));
    }
    return new earl::value::List(std::move(list));
}

earl::value::Obj *eval_expr_get2(ExprGet *expr, Ctx &ctx) {
    earl::value::Obj *result = nullptr;

    earl::value::Obj *left = Interpreter::eval_expr(expr->m_left.get(), ctx);

    if (expr->get_type() != ExprType::Term) {
        ERR(Err::Type::Fatal, "cannot use `get` expression on non-terminal expression");
    }

    ExprTerm *right = dynamic_cast<ExprTerm *>(expr->m_right.get());

    switch (right->get_term_type()) {
    case ExprTermType::Ident: {
        UNIMPLEMENTED("eval_expr_get::ExprTermType::Ident");
    } break;
    case ExprTermType::Func_Call: {
        ExprFuncCall *func_expr = dynamic_cast<ExprFuncCall *>(right);
        const std::string &id = func_expr->m_id->lexeme();
        std::vector<earl::value::Obj *> params;

        std::for_each(func_expr->m_params.begin(), func_expr->m_params.end(), [&](auto &e) {
            params.push_back(Interpreter::eval_expr(e.get(), ctx));
        });

        // Check if class
        if (left->type() == earl::value::Type::Class) {
            auto *klass = dynamic_cast<earl::value::Class *>(left);
            auto *method = klass->get_method(id);
            return eval_user_defined_class_method(method, params, klass, ctx);
        }

        // Not a class, it is an intrinsic
        else if (Intrinsics::is_member_intrinsic(id)) {
            result = Intrinsics::call_member(id, left, params, ctx);
        }

        else {
            assert(false && "invalid getter operation `.`");
        }

    } break;
    default: {
        ERR_WARGS(Err::Type::Fatal,
                  "unknown `get` term type (%d)",
                  static_cast<int>(right->get_term_type()));
    } break;
    }

    return result;
}

earl::value::Obj *eval_expr_get(ExprGet *expr, Ctx &ctx) {
    earl::value::Obj *left = Interpreter::eval_expr(expr->m_left.get(), ctx);

    if (expr->get_type() != ExprType::Term) {
        ERR(Err::Type::Fatal, "cannot use `get` expression on non-terminal expression");
    }

    if (left->type() == earl::value::Type::Module) {
        auto *mod = dynamic_cast<earl::value::Module *>(left);
        return Interpreter::eval_expr(expr->m_right.get(), *mod->value());
    }
    else {
        return eval_expr_get2(expr, ctx);
    }

    return nullptr;
}

earl::value::Obj *eval_expr_array_access(ExprArrayAccess *expr, Ctx &ctx) {
    earl::value::Obj *result = nullptr;

    earl::value::Obj *left = Interpreter::eval_expr(expr->m_left.get(), ctx);
    earl::value::Obj *idx = Interpreter::eval_expr(expr->m_expr.get(), ctx);

    switch (left->type()) {
    case earl::value::Type::List: {
        earl::value::List *list = dynamic_cast<earl::value::List *>(left);
        result = list->nth(idx);
    } break;
    case earl::value::Type::Str: {
        UNIMPLEMENTED("eval_expr_array_access: earl::value::Type::Str");
    } break;
    default: {
        ERR(Err::Type::Fatal, "cannot use `[]` operator on non-list type");
    } break;
    }

    return result;
}

earl::value::Obj *eval_expr_term(ExprTerm *expr, Ctx &ctx) {
    switch (expr->get_term_type()) {
    case ExprTermType::Ident: {
        ctx.debug_dump();

        ExprIdent *ident = dynamic_cast<ExprIdent *>(expr);

        // Check for a module
        earl::value::Module *mod = ctx.get_registered_module(ident->m_tok->lexeme());
        if (mod)
            return mod;

        // Not a module, find the variable
        earl::variable::Obj *stored = ctx.get_registered_variable(ident->m_tok->lexeme());
        return stored->value();
    } break;
    case ExprTermType::Int_Literal: {
        ExprIntLit *intlit = dynamic_cast<ExprIntLit *>(expr);
        return new earl::value::Int(std::stoi(intlit->m_tok->lexeme()));
    } break;
    case ExprTermType::Str_Literal: {
        ExprStrLit *strlit = dynamic_cast<ExprStrLit *>(expr);
        return new earl::value::Str(strlit->m_tok->lexeme());
    } break;
    case ExprTermType::Func_Call: {
        return eval_expr_funccall(dynamic_cast<ExprFuncCall *>(expr), ctx);
    } break;
    case ExprTermType::List_Literal: {
        return eval_expr_list_literal(dynamic_cast<ExprListLit *>(expr), ctx);
    } break;
    case ExprTermType::Get: {
        ExprGet *get = dynamic_cast<ExprGet *>(expr);
        return eval_expr_get(get, ctx);
    }
    case ExprTermType::Array_Access: {
        ExprArrayAccess *access = dynamic_cast<ExprArrayAccess *>(expr);
        return eval_expr_array_access(access, ctx);
    } break;
    default: {
        ERR_WARGS(Err::Type::Fatal, "unknown expression term type %d", static_cast<int>(expr->get_term_type()));
    } break;
    }
}

earl::value::Obj *eval_expr_bin(ExprBinary *expr, Ctx &ctx) {
    earl::value::Obj *lhs = Interpreter::eval_expr(expr->m_lhs.get(), ctx);
    earl::value::Obj *rhs = Interpreter::eval_expr(expr->m_rhs.get(), ctx);

    earl::value::Obj *result = lhs->binop(expr->m_op.get(), rhs);

    return result;
}

earl::value::Obj *Interpreter::eval_expr(Expr *expr, Ctx &ctx) {
    switch (expr->get_type()) {
    case ExprType::Term: {
        return eval_expr_term(dynamic_cast<ExprTerm *>(expr), ctx);
    } break;
    case ExprType::Binary: {
        return eval_expr_bin(dynamic_cast<ExprBinary *>(expr), ctx);
    } break;
    default: {
        ERR_WARGS(Err::Type::Fatal, "unknown expr type %d", static_cast<int>(expr->get_type()));
    } break;
    }
}

earl::value::Obj *eval_stmt_expr(StmtExpr *stmt, Ctx &ctx) {
    return Interpreter::eval_expr(stmt->m_expr.get(), ctx);
}

earl::value::Obj *eval_stmt_block(StmtBlock *block, Ctx &ctx) {
    earl::value::Obj *result = nullptr;

    ctx.push_scope();
    for (size_t i = 0; i < block->m_stmts.size(); ++i) {
        result = eval_stmt(block->m_stmts.at(i).get(), ctx);
        if (result && result->type() != earl::value::Type::Void)
            break;
    }
    ctx.pop_scope();

    return result;
}

// When we hit a statement `def` (a function declaration),
// we do not actually want to execute this function.
// We just want to add it to the global context so it
// can be called later from either a statement expression
// or a right-hand-side assignment.
earl::value::Obj *eval_stmt_def(StmtDef *stmt, Ctx &ctx) {
    if (ctx.function_is_registered(stmt->m_id->lexeme())) {
        ERR_WARGS(Err::Type::Redeclared,
                  "function `%s` is already declared", stmt->m_id->lexeme().c_str());
    }

    earl::function::Obj *created_function = new earl::function::Obj(stmt, &stmt->m_args);
    ctx.register_function(created_function);
    return new earl::value::Void();
}

earl::value::Obj *eval_stmt_if(StmtIf *stmt, Ctx &ctx) {
    earl::value::Obj *expr_result = Interpreter::eval_expr(stmt->m_expr.get(), ctx);
    earl::value::Obj *result = nullptr;

    if (expr_result->boolean()) {
        result = eval_stmt_block(stmt->m_block.get(), ctx);
    }
    else if (stmt->m_else.has_value()) {
        result = eval_stmt_block(stmt->m_else.value().get(), ctx);
    }

    delete expr_result;

    return result;
}

earl::value::Obj *eval_stmt_return(StmtReturn *stmt, Ctx &ctx) {
    return Interpreter::eval_expr(stmt->m_expr.get(), ctx);
}

earl::value::Obj *eval_stmt_mut(StmtMut *stmt, Ctx &ctx) {
    earl::value::Obj *left = Interpreter::eval_expr(stmt->m_left.get(), ctx);
    earl::value::Obj *right = Interpreter::eval_expr(stmt->m_right.get(), ctx);

    left->mutate(right);

    return new earl::value::Void();
}

earl::value::Obj *eval_stmt_while(StmtWhile *stmt, Ctx &ctx) {
    earl::value::Obj *expr_result = nullptr;
    earl::value::Obj *result = nullptr;

    while ((expr_result = Interpreter::eval_expr(stmt->m_expr.get(), ctx))->boolean()) {
        result = eval_stmt_block(stmt->m_block.get(), ctx);
        if (result && result->type() != earl::value::Type::Void)
            break;
        delete expr_result;
    }

    return result;
}

earl::value::Obj *eval_stmt_for(StmtFor *stmt, Ctx &ctx) {
    earl::value::Obj *result = nullptr;
    earl::value::Obj *start_expr = Interpreter::eval_expr(stmt->m_start.get(), ctx);
    earl::value::Obj *end_expr = Interpreter::eval_expr(stmt->m_end.get(), ctx);

    earl::variable::Obj *enumerator
        = new earl::variable::Obj(stmt->m_enumerator.get(), std::unique_ptr<earl::value::Obj>(start_expr));

    assert(!ctx.variable_is_registered(enumerator->id()));
    ctx.register_variable(enumerator);

    earl::value::Int *start = dynamic_cast<earl::value::Int *>(start_expr);
    earl::value::Int *end = dynamic_cast<earl::value::Int *>(end_expr);

    while (start->value() < end->value()) {
        result = eval_stmt_block(stmt->m_block.get(), ctx);

        if (result && result->type() != earl::value::Type::Void)
            break;

        start->mutate(new earl::value::Int(start->value() + 1));
    }

    ctx.unregister_variable(enumerator->id());

    delete start_expr;
    delete end_expr;

    return result;
}

earl::value::Obj *eval_stmt_class(StmtClass *stmt, Ctx &ctx) {
    ctx.available_classes.push_back(stmt);
    return new earl::value::Void();
}

earl::value::Obj *eval_stmt(Stmt *stmt, Ctx &ctx) {
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
    case StmtType::Class: {
        return eval_stmt_class(dynamic_cast<StmtClass *>(stmt), ctx);
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
    case StmtType::Mod: {
        StmtMod *mod = dynamic_cast<StmtMod *>(stmt);
        ctx.set_module(std::move(mod->m_id));
        return new earl::value::Void();
    } break;
    case StmtType::Import: {
        StmtImport *im = dynamic_cast<StmtImport *>(stmt);

        std::vector<std::string> keywords = COMMON_EARLKW_ASCPL;
        std::vector<std::string> types    = COMMON_EARLTY_ASCPL;
        std::string comment               = COMMON_EARL_COMMENT;

        std::unique_ptr<Lexer> lexer      = lex_file(im->m_fp.get()->lexeme().c_str(), keywords, types, comment);
        std::unique_ptr<Program> program  = Parser::parse_program(*lexer.get());

        Ctx *child_ctx = Interpreter::interpret(std::move(program), std::move(lexer));
        child_ctx->m_parent = &ctx;
        ctx.push_child_context(std::unique_ptr<Ctx>(std::move(child_ctx)));
        return new earl::value::Void();
    } break;
    default:
        assert(false && "eval_stmt: invalid statement");
    }
}

Ctx *Interpreter::interpret(std::unique_ptr<Program> program, std::unique_ptr<Lexer> lexer) {
    Ctx *ctx = new Ctx(std::move(lexer), std::move(program));
    earl::value::Obj *meta;

    ctx->m_parent = nullptr;

    for (size_t i = 0; i < ctx->stmts_len(); ++i) {
        meta = eval_stmt(ctx->get_stmt(i), *ctx);
        delete meta;
    }

    return std::move(ctx);
}
