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

#include <cassert>

#include "earl.hpp"
#include "err.hpp"
#include "utils.hpp"
#include "common.hpp"
#include "ctx.hpp"
#include "interpreter.hpp"

using namespace earl::value;

Closure::Closure(ExprClosure *expr_closure, std::vector<std::pair<Token *, uint32_t>> params)
    : m_expr_closure(expr_closure), m_params(std::move(params)) {}

StmtBlock *Closure::block(void) {
    UNIMPLEMENTED("Closure::call");
}

void Closure::load_parameters(std::vector<earl::value::Obj *> &values, Ctx &ctx) {
    (void)values;
    (void)ctx;
    UNIMPLEMENTED("Closure::load_parameters");
}

std::shared_ptr<Obj> Closure::call(std::vector<std::shared_ptr<earl::value::Obj>> &values, Ctx &ctx) {
    (void)values;
    (void)ctx;
    UNIMPLEMENTED("Closure::call");
}

bool Closure::has_local(const std::string &id) {
    (void)id;
    UNIMPLEMENTED("Closure::call");
}

/*** OVERRIDES ***/
Type Closure::type(void) const {
    return Type::Closure;
}

std::shared_ptr<Obj> Closure::binop(Token *op, std::shared_ptr<Obj> &other) {
    (void)op;
    (void)other;
    UNIMPLEMENTED("Closure::binop");
}

bool Closure::boolean(void) {
    UNIMPLEMENTED("Closure::boolean");
}

void Closure::mutate(const std::shared_ptr<Obj> &other) {
    (void)other;
    UNIMPLEMENTED("Closure::mutate");
}

std::shared_ptr<Obj> Closure::copy(void) {
    UNIMPLEMENTED("Closure::copy");
}

bool Closure::eq(std::shared_ptr<Obj> &other) {
    (void)other;
    UNIMPLEMENTED("Closure::eq");
    return true;
}

std::string Closure::to_cxxstring(void) {
    ERR(Err::Type::Fatal, "unable to convert `closure` type to a string");
}
