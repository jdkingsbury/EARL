#include <cassert>
#include <memory>

#include "ast.hpp"

/*** PROGRAM ***/

Program::Program(std::vector<std::unique_ptr<Stmt>> stmts)
  : m_stmts(std::move(stmts)) {}

/*** EXPRESSIONS ***/

ExprFuncCall::ExprFuncCall(std::unique_ptr<Token> id,
                           std::vector<std::unique_ptr<Token>> params)
  : m_id(std::move(id)), m_params(std::move(params)) {}

ExprType ExprFuncCall::get_type() const {
  return ExprType::Func_Call;
}

ExprBinary::ExprBinary(std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
  : m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}

ExprType ExprBinary::get_type() const {
  return ExprType::Binary;
}

/*** TERM EXPRESSIONS ***/

ExprStrLit::ExprStrLit(std::unique_ptr<Token> tok) : m_tok(std::move(tok)) {}

ExprType ExprStrLit::get_type() const {
  return ExprType::Term;
}

ExprTermType ExprStrLit::get_term_type() const {
  return ExprTermType::Str_Literal;
}

const Token &ExprStrLit::get_tok() const {
  assert(false && "unimplemented");
}

ExprIntLit::ExprIntLit(std::unique_ptr<Token> tok) : m_tok(std::move(tok)) {}

ExprType ExprIntLit::get_type() const {
  return ExprType::Term;
}

ExprTermType ExprIntLit::get_term_type() const {
  return ExprTermType::Int_Literal;
}

const Token &ExprIntLit::get_tok() const {
  assert(false && "unimplemented");
}

ExprIdent::ExprIdent(std::unique_ptr<Token> tok) : m_tok(std::move(tok)) {}

ExprType ExprIdent::get_type() const {
  return ExprType::Term;
}

ExprTermType ExprIdent::get_term_type() const {
  return ExprTermType::Ident;
}

const Token &ExprIdent::get_tok() const {
  assert(false && "unimplemented");
}

/*** STATEMENTS ***/

StmtDef::StmtDef(std::unique_ptr<Token> id,
                 std::vector<std::pair<std::unique_ptr<Token>, std::unique_ptr<Token>>> args,
                 std::unique_ptr<Token> rettype,
                 std::unique_ptr<StmtBlock> block) :
    m_id(std::move(id)), m_args(std::move(args)),
    m_rettype(std::move(rettype)), m_block(std::move(block)) {}

StmtType StmtDef::stmt_type() const {
  return StmtType::Def;
}

StmtLet::StmtLet(std::unique_ptr<Token> id,
                 std::unique_ptr<Token> type,
                 std::unique_ptr<Expr> expr)
  : m_id(std::move(id)), m_type(std::move(type)), m_expr(std::move(expr)) {}

StmtType StmtLet::stmt_type() const {
  return StmtType::Let;
}

StmtBlock::StmtBlock(std::vector<std::unique_ptr<Stmt>> stmts) : m_stmts(std::move(stmts)) {}

void add_stmt(std::unique_ptr<Stmt> stmt) {
  assert(false && "unimplemented");
}

const std::vector<std::unique_ptr<Stmt>> &StmtBlock::get_stmts() const {
  assert(false && "unimplemented");
}

StmtType StmtBlock::stmt_type() const {
  return StmtType::Block;
}

StmtMut::StmtMut(std::unique_ptr<Expr> left,
                 std::unique_ptr<Token> op,
                 std::unique_ptr<Expr> right)
  : m_left(std::move(left)), m_op(std::move(op)), m_right(std::move(right)) {}

StmtType StmtMut::stmt_type() const {
  return StmtType::Mut;
}

StmtExpr::StmtExpr(std::unique_ptr<Expr> expr) : m_expr(std::move(expr)) {}

StmtType StmtExpr::stmt_type() const {
  return StmtType::Stmt_Expr;
}
