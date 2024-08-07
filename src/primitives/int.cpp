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

Int::Int(int value) : m_value(value) {}

int
Int::value(void) {
    return m_value;
}

void
Int::fill(int value) {
    m_value = value;
}

Type
Int::type(void) const {
    return Type::Int;
}

std::shared_ptr<Obj>
Int::binop(Token *op, std::shared_ptr<Obj> &other) {
    if (!type_is_compatable(this, other.get())) {
        assert(false && "cannot binop (fix this message)");
    }

    switch (op->type()) {
    case TokenType::Plus: {
        return other->type() == Type::Float ?
            std::make_shared<Int>(this->value() + dynamic_cast<Float *>(other.get())->value()) :
            std::make_shared<Int>(this->value() + dynamic_cast<Int *>(other.get())->value());
    } break;
    case TokenType::Minus: {
        return other->type() == Type::Float ?
            std::make_shared<Int>(this->value() - dynamic_cast<Float *>(other.get())->value()) :
            std::make_shared<Int>(this->value() - dynamic_cast<Int *>(other.get())->value());
    } break;
    case TokenType::Asterisk: {
        return other->type() == Type::Float ?
            std::make_shared<Int>(this->value() * dynamic_cast<Float *>(other.get())->value()) :
            std::make_shared<Int>(this->value() * dynamic_cast<Int *>(other.get())->value());
    } break;
    case TokenType::Forwardslash: {
        return other->type() == Type::Float ?
            std::make_shared<Int>(this->value() / dynamic_cast<Float *>(other.get())->value()) :
            std::make_shared<Int>(this->value() / dynamic_cast<Int *>(other.get())->value());
    } break;
    case TokenType::Percent: {
        if (other->type() == Type::Float)
            ERR(Err::Type::Fatal, "cannot use module `%%` with floating point");
        return std::make_shared<Int>(this->value() % dynamic_cast<Int *>(other.get())->value());
    } break;
    case TokenType::Lessthan: {
        return other->type() == Type::Float ?
            std::make_shared<Bool>(this->value() < dynamic_cast<Float *>(other.get())->value()) :
            std::make_shared<Bool>(this->value() < dynamic_cast<Int *>(other.get())->value());
    } break;
    case TokenType::Greaterthan: {
        return other->type() == Type::Float ?
            std::make_shared<Bool>(this->value() > dynamic_cast<Float *>(other.get())->value()) :
            std::make_shared<Bool>(this->value() > dynamic_cast<Int *>(other.get())->value());
    } break;
    case TokenType::Double_Equals: {
        return other->type() == Type::Float ?
            std::make_shared<Bool>(this->value() == dynamic_cast<Float *>(other.get())->value()) :
            std::make_shared<Bool>(this->value() == dynamic_cast<Int *>(other.get())->value());
    } break;
    case TokenType::Greaterthan_Equals: {
        return other->type() == Type::Float ?
            std::make_shared<Bool>(this->value() >= dynamic_cast<Float *>(other.get())->value()) :
            std::make_shared<Bool>(this->value() >= dynamic_cast<Int *>(other.get())->value());
    } break;
    case TokenType::Lessthan_Equals: {
        return other->type() == Type::Float ?
            std::make_shared<Bool>(this->value() <= dynamic_cast<Float *>(other.get())->value()) :
            std::make_shared<Bool>(this->value() <= dynamic_cast<Int *>(other.get())->value());
    } break;
    case TokenType::Bang_Equals: {
        return other->type() == Type::Float ?
            std::make_shared<Bool>(this->value() != dynamic_cast<Float *>(other.get())->value()) :
            std::make_shared<Bool>(this->value() != dynamic_cast<Int *>(other.get())->value());
    } break;
    case TokenType::Double_Pipe: {
        return other->type() == Type::Float ?
            std::make_shared<Bool>(this->value() || dynamic_cast<Float *>(other.get())->value()) :
            std::make_shared<Bool>(this->value() || dynamic_cast<Int *>(other.get())->value());
    } break;
    default: {
        Err::err_wtok(op);
        ERR(Err::Type::Fatal, "invalid binary operator");
    }
    }
}

bool
Int::boolean(void) {
    return this->value();
}

void
Int::mutate(const std::shared_ptr<Obj> &other) {
    if (!type_is_compatable(this, other.get())) {
        assert(false && "cannot mutate (fix this message)");
    }

    switch (other->type()) {
    case Type::Int: {
        m_value = dynamic_cast<Int *>(other.get())->value();
    } break;
    default: {
        assert(false && "unreachable");
    }
    }
}

std::shared_ptr<Obj>
Int::copy(void) {
    return std::make_shared<Int>(m_value);
}

bool
Int::eq(std::shared_ptr<Obj> &other) {
    if (other->type() == Type::Void)
        return true;

    if (other->type() != Type::Int)
        return false;
    return this->value() == dynamic_cast<Int *>(other.get())->value();
}

std::string
Int::to_cxxstring(void) {
    return std::to_string(m_value);
}
