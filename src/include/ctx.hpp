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

/**
 * Provides a 'Context' struct that
 * holds all relevant information
 * that a runtime needs.
 */

#ifndef CTX_H
#define CTX_H

#include <vector>
#include <unordered_map>

#include "earl.hpp"
#include "shared-scope.hpp"

enum class CtxType {
    World,
    Function,
    Class,
};

struct Ctx {
    virtual CtxType type(void) const = 0;
    virtual void push_variable_scope(void) = 0;
    virtual void pop_variable_scope(void) = 0;
    virtual void add_variable(std::shared_ptr<earl::variable::Obj> var) = 0;

private:
    SharedScope<std::string, earl::variable::Obj> m_scope;
};

struct WorldCtx : public Ctx {
    WorldCtx() = default;
    ~WorldCtx() = default;

    CtxType type(void) const override;
    void push_variable_scope(void) override;
    void pop_variable_scope(void) override;
    void add_variable(std::shared_ptr<earl::variable::Obj> var) override;

private:
    std::string m_name;
    std::vector<std::shared_ptr<Ctx>> m_imports;
};

struct FunctionCtx : public Ctx {
    FunctionCtx() = default;
    ~FunctionCtx() = default;

    CtxType type(void) const override;
    void push_variable_scope(void) override;
    void pop_variable_scope(void) override;
    void add_variable(std::shared_ptr<earl::variable::Obj> var) override;

private:
    std::shared_ptr<Ctx> m_owner;
};

struct ClassCtx : public Ctx {
    ClassCtx() = default;
    ~ClassCtx() = default;

    CtxType type(void) const override;
    void push_variable_scope(void) override;
    void pop_variable_scope(void) override;
    void add_variable(std::shared_ptr<earl::variable::Obj> var) override;

private:
    std::shared_ptr<Ctx> m_owner;
};

#endif // CTX_H
