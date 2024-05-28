#include <cassert>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <any>

#include "interpreter.hpp"
#include "token.hpp"
#include "ast.hpp"
#include "common.hpp"

#define earlty_to_cppty(ty) ty == EarlTy::Int ? int : std::string

enum class EarlTy {
  Int,
  Str,
};

struct EarlVar {
  std::unique_ptr<Token> m_id;
  EarlTy m_type;
  std::any m_value;

  uint32_t m_refcount;

  EarlVar(std::unique_ptr<Token> id, EarlTy type,
          std::any value = nullptr, uint32_t refcount = 1)
    : m_id(std::move(id)),
      m_type(type),
      m_value(value),
      m_refcount(refcount) {}
};

struct Ctx {
  std::vector<std::unordered_map<std::string, EarlVar>> m_scope;
  std::unordered_map<EarlTy, std::vector<EarlTy>> m_earl_compat_tys;

  Ctx() {
    m_scope.emplace_back();

    m_earl_compat_tys[EarlTy::Int] = {EarlTy::Int};
    m_earl_compat_tys[EarlTy::Str] = {EarlTy::Str};
  }

  ~Ctx() = default;

  bool iscompat_type(EarlTy ty1, EarlTy ty2) {
    for (EarlTy ty : m_earl_compat_tys[ty1]) {
      if (ty == ty2) {
        return true;
      }
    }
    return false;
  }

  void add_earlvar(std::unique_ptr<Token> id, EarlTy type, std::any value = nullptr) {
    std::string name = id->lexeme();
    m_scope.back().emplace(name, EarlVar(std::move(id), type, std::move(value)));
  }

  bool has_earlvar(const std::string &id) const {
    for (auto it = m_scope.rbegin(); it != m_scope.rend(); ++it) {
      if (it->find(id) != it->end()) {
        return true;
      }
    }
    return false;
  }

  EarlVar &get_earlvar(const std::string &id) {
    for (auto it = m_scope.rbegin(); it != m_scope.rend(); ++it) {
      if (it->find(id) != it->end()) {
        return it->at(id);
      }
    }
    assert(false && "get_var: variable not found");
  }
};

static EarlTy ty_to_earlty(TokenType ty) {
  switch (ty) {
  case TokenType::Intlit:
    return EarlTy::Int;
  case TokenType::Strlit:
    return EarlTy::Str;
  default:
    assert(false && "ty_to_earlty: unknown type");
  }
}

void debug_dump_scope(Ctx &ctx) {
  std::cout << "Dumping scope\n";
  for (auto &scope : ctx.m_scope) {
    for (auto &var : scope) {
      switch (var.second.m_type) {
      case EarlTy::Int: // TODO: add type checking
        std::cout << var.first << " = " << std::any_cast<int>(var.second.m_value) << std::endl;
        break;
      default:
        std::cerr << "error: unknown type" << std::endl;
        break;
      }
    }
  }
}

void scope_pop(Ctx &ctx) {
  ctx.m_scope.pop_back();
}

void scope_push(Ctx &ctx) {
  ctx.m_scope.emplace_back();
}

static std::any eval_expr(Expr *, Ctx &);

static std::any eval_expr_term(ExprTerm *expr, Ctx &ctx) {
  switch (expr->get_term_type()) {
  case ExprTermType::Ident: {
    ExprIdent *expr_ident = dynamic_cast<ExprIdent *>(expr);
    if (!ctx.has_earlvar(expr_ident->tok().lexeme())) {
      std::cerr << "error: variable '" << expr_ident->tok().lexeme() << "' not declared" << std::endl;
      return nullptr;
    }
    // return ctx.get_earlvar(expr_ident->tok().lexeme()).m_value;
    return &ctx.get_earlvar(expr_ident->tok().lexeme());
  } break;
  case ExprTermType::Int_Literal: {
    ExprIntLit *expr_intlit = dynamic_cast<ExprIntLit *>(expr);
    return std::stoi(expr_intlit->tok().lexeme());
  } break;
  case ExprTermType::Str_Literal: {
    assert(false && "eval_expr_term: string literals not implemented");
  } break;
  default:
    assert(false && "eval_expr_term: unknown term type");
  }
}

static std::any eval_expr_binary(ExprBinary *expr, Ctx &ctx) {
  std::any lhs = eval_expr(&expr->lhs(), ctx);
  std::any rhs = eval_expr(&expr->rhs(), ctx);

  // TODO: type checking and different types
  switch (expr->op().type()) {
  case TokenType::Plus:
    return std::any_cast<int>(lhs) + std::any_cast<int>(rhs);
  case TokenType::Minus:
    return std::any_cast<int>(lhs) - std::any_cast<int>(rhs);
  case TokenType::Asterisk:
    return std::any_cast<int>(lhs) * std::any_cast<int>(rhs);
  case TokenType::Forwardslash:
    return std::any_cast<int>(lhs) / std::any_cast<int>(rhs);
  default:
    assert(false && "eval_expr_binary: unknown operator");
  }
}

static std::any eval_expr(Expr *expr, Ctx &ctx) {
  switch (expr->get_type()) {
  case ExprType::Term: {
    return eval_expr_term(dynamic_cast<ExprTerm *>(expr), ctx);
  } break;
  case ExprType::Binary: {
    return eval_expr_binary(dynamic_cast<ExprBinary *>(expr), ctx);
  }
  case ExprType::Func_Call:
    assert(false && "todo");
  default:
    assert(false && "eval_expr: unknown expression");
  }
}

static void eval_stmt_mut(StmtMut *stmt, Ctx &ctx) {
  std::any left = eval_expr(&stmt->left(), ctx);
  std::any right = eval_expr(&stmt->right(), ctx);

  if (left.type() == typeid(EarlVar *)) {
    EarlVar *earlvar = std::any_cast<EarlVar *>(left);
    earlvar->m_value = right;
  }
  else {
    std::cerr << "error: left side of assignment is not a variable" << std::endl;
  }
}

static EarlTy gen_earlty(Token *tok) {
  (void)tok;
  return EarlTy::Int;
}

static void eval_stmt_let(StmtLet *stmt, Ctx &ctx) {
  if (ctx.has_earlvar(stmt->id().lexeme())) {
    std::cerr << "error: variable '" << stmt->id().lexeme() << "' already declared" << std::endl;
    return;
  }

  std::any value = eval_expr(&stmt->expr(), ctx);

  if (value.type() == typeid(EarlVar *)) {
    value = std::any_cast<EarlVar *>(value)->m_value;
    ctx.add_earlvar(std::move(stmt->m_id), gen_earlty(stmt->m_type.get()), /*value=*/value);
  }
  else {
    ctx.add_earlvar(std::move(stmt->m_id), gen_earlty(stmt->m_type.get()), /*value=*/value);
  }
}

static void eval_stmt(std::unique_ptr<Stmt> stmt, Ctx &ctx)
{
  switch (stmt->stmt_type()) {
  case StmtType::Let: {
    if (auto stmt_let = dynamic_cast<StmtLet *>(stmt.get())) {
      eval_stmt_let(stmt_let, ctx);
    } else {
      assert(false && "eval_stmt (let): invalid stmt type");
    }
    break;
  }
  case StmtType::Mut: {
    if (auto stmt_mut = dynamic_cast<StmtMut *>(stmt.get())) {
      eval_stmt_mut(stmt_mut, ctx);
    } else {
      assert(false && "eval_stmt (mut): invalid stmt type");
    }
  } break;
  default:
    assert(false && "eval_stmt: unknown stmt type");
  }
}

void interpret(Program &program)
{
  (void)ty_to_earlty;

  Ctx ctx;

  for (size_t i = 0; i < program.stmts_len(); ++i) {
    eval_stmt(std::move(program.get_stmt(i)), ctx);
  }

  debug_dump_scope(ctx);
}
