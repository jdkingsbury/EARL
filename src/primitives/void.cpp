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

#include <memory>

#include "earl.hpp"
#include "err.hpp"
#include "utils.hpp"

using namespace earl::value;

Void::Void(void *value) : m_value(value) {}

Type
Void::type(void) const {
    return Type::Void;
}

std::shared_ptr<Obj>
Void::binop(Token *op, std::shared_ptr<Obj> &other) {
    switch (op->type()) {
    case TokenType::Double_Equals: return std::make_shared<Bool>(this->eq(other));
    case TokenType::Bang_Equals: return std::make_shared<Bool>(!this->eq(other));
    default:
        Err::err_wtok(op);
        std::string msg = "invalid operator for binary operation `"+op->lexeme()+"` on unit type";
        throw InterpreterException(msg);
    }
}

bool
Void::boolean(void) {
    return false;
}

void
Void::mutate(const std::shared_ptr<Obj> &other, StmtMut *stmt) {
    ASSERT_MUTATE_COMPAT(this, other.get(), stmt);
    ASSERT_CONSTNESS(this, stmt);
}

std::shared_ptr<Obj>
Void::copy(void) {
    return std::make_shared<Void>();
}

bool
Void::eq(std::shared_ptr<Obj> &other) {
    return other->type() == Type::Void;
}

std::string
Void::to_cxxstring(void) {
    return "<unit>";
}

void
Void::spec_mutate(Token *op, const std::shared_ptr<Obj> &other, StmtMut *stmt) {
    (void)other;
    Err::err_wtok(op);
    std::string msg = "invalid operator for special mutation `"+op->lexeme()+"` on unit type";
    throw InterpreterException(msg);
}

std::shared_ptr<Obj>
Void::unaryop(Token *op) {
    (void)op;
    Err::err_wtok(op);
    std::string msg = "invalid unary operator on unit type";
    throw InterpreterException(msg);
    return nullptr; // unreachable
}

void
Void::set_const(void) {
    m_const = true;
}


