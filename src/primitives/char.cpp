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
#include <memory>

#include "earl.hpp"
#include "err.hpp"
#include "utils.hpp"

using namespace earl::value;

Char::Char(std::string value) {
    if (value.size() == 0) {
        m_value = '\0';
    }
    else {
        m_value = value[0];
    }
}

char Char::value(void) {
    return m_value;
}

Type Char::type(void) const {
    return Type::Char;
}

std::shared_ptr<Obj> Char::binop(Token *op, std::shared_ptr<Obj> &other) {
    (void)other;
    (void)op;
    UNIMPLEMENTED("Char::binop");
}

bool Char::boolean(void) {
    UNIMPLEMENTED("Char::boolean");
}

void Char::mutate(std::shared_ptr<Obj> &other) {
    (void)other;
    UNIMPLEMENTED("Char::mutate");
}

std::shared_ptr<Obj> Char::copy(void) {
    UNIMPLEMENTED("Char::copy");
}

bool Char::eq(std::shared_ptr<Obj> &other) {
    (void)other;
    UNIMPLEMENTED("Char::eq");
}

std::string Char::to_cxxstring(void) {
    return std::string(1, m_value);
}
