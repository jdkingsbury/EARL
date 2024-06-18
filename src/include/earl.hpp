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

#ifndef EARL_H
#define EARL_H

#include <unordered_map>
#include <string>
#include <vector>
#include <optional>
#include <variant>

#include "scope.hpp"
#include "ast.hpp"
#include "token.hpp"

/**
 * All values associated with EARL during runtime.
 */
namespace earl {

    /**
     * The values that are returned from each
     * evaluation during runtime. They represent
     * the underlying data of EARL.
     */
    namespace value {

        /// @brief The intrinsic types of EARL
        enum class Type {
            /** EARL 32bit integer type */
            Int,
            /** EARL string type */
            Str,
            /** EARL UNIT type (used when eval's don't return anything) */
            Void,
            /** EARL list type (holds any value including a mix of datatypes) */
            List,
        };

        /// @brief The base abstract class that all
        /// EARL values inherit from
        struct Obj {
            virtual ~Obj() {}

            /// @brief Get the type of the value
            virtual Type type(void) const = 0;

            /// @brief Perform a binary operation on THIS
            /// value with another value
            /// @param op The binary operator
            /// @param other The object to perform the binop with
            virtual Obj *binop(Token *op, Obj *other) = 0;

            /// @brief Get the evaluation when put into
            /// a conditional to evaluate to either true or false
            virtual bool boolean(void) = 0;

            /// @brief Modify the underlying data of THIS value
            /// with the underlying value of another object
            /// @param other The value to mutate THIS instance with
            virtual void mutate(Obj *other) = 0;

            /// @brief Copy THIS instance
            virtual Obj *copy(void) = 0;
        };

        /// @brief The structure that represents EARL 32bit integers
        struct Int : public Obj {
            Int(int value = 0);

            /// @brief Fill the underlying data with some data
            /// @param value The value to use to fill
            void fill(int value);

            /// @brief Get the underlying integer value
            int value(void);

            /*** OVERRIDES ***/
            Type type(void) const             override;
            Obj *binop(Token *op, Obj *other) override;
            bool boolean(void)                override;
            void mutate(Obj *other)           override;
            Obj *copy(void)                   override;

        private:
            int m_value;
        };

        /// @brief The structure that represents EARL strings
        struct Str : public Obj {
            Str(std::string value = "");

            /// @brief Fill the underlying data with some data
            /// @param value The value to use to fill
            void fill(std::string value);

            /// @brief Get the underlying string value
            std::string &value(void);

            /*** OVERRIDES ***/
            Type type(void) const             override;
            Obj *binop(Token *op, Obj *other) override;
            bool boolean(void)                override;
            void mutate(Obj *other)           override;
            Obj *copy(void)                   override;

        private:
            std::string m_value;
        };

        /// @brief The structure that represents EARL UNITs
        struct Void : public Obj {
            Void(void *value = nullptr);

            [[deprecated]]
            void *value(void);

            /*** OVERRIDES ***/
            Type type(void) const             override;
            Obj *binop(Token *op, Obj *other) override;
            bool boolean(void)                override;
            void mutate(Obj *other)           override;
            Obj *copy(void)                   override;

        private:
            void *m_value;
        };

        /// @brief The structure that represents EARL lists.
        /// They can hold any value in any mix of them i.e.,
        /// list = [int, str, str, int, list[int, str]]
        struct List : public Obj {
            List(std::vector<Obj *> value = {});

            /// @brief Fill the underlying data with some data
            /// @param value The value to use to fill
            void fill(std::vector<Obj *> value);

            /// @brief Get the underlying list value
            std::vector<Obj *> &value(void);

            /// @brief Get the `nth` element from the list
            /// @note This is called from the intrinsic `nth` member function
            /// @param idx The object that contains the index
            /// @note `idx` MUST BE an integer value
            Obj *nth(Obj *idx);

            Obj *rev(void);

            Obj *append(std::vector<Obj *> &values);

            /*** OVERRIDES ***/
            Type type(void) const             override;
            Obj *binop(Token *op, Obj *other) override;
            bool boolean(void)                override;
            void mutate(Obj *other)           override;
            Obj *copy(void)                   override;

        private:
            std::vector<Obj *> m_value;
        };

        /// @brief Get an empty EARL value from a type
        /// @param s The string type to parse
        [[deprecated]] [[nodiscard]]
        Obj *of_str(const std::string &s);

        /// @brief Check if two EARL values are type compatable
        /// @param obj1 The first object
        /// @param obj2 The second object
        [[nodiscard]]
        bool type_is_compatable(Obj *const obj1, Obj *const obj2);
    };

    /**
     * How variables are created in EARL during runtime
     */
    namespace variable {

        /// @brief The structure to represent EARL variables
        struct Obj {
            Obj(Token *id, std::unique_ptr<value::Obj> value);
            ~Obj() = default;

            /// @brief Get the identifier of this variable
            const std::string &id(void) const;

            /// @brief Check if a variable is in global scope
            [[deprecated]]
            bool is_global(void) const;

            /// @brief Get the actual value of this variable
            value::Obj *value(void) const;

            /// @brief Get the type of this variable
            value::Type type(void) const;

        private:
            Token *m_id;
            std::unique_ptr<value::Obj> m_value;
        };
    };

    /**
     * How functions are created in EARL during runtime
     */
    namespace function {

        /// @brief The structure to represent EARL functions
        struct Obj {
            Obj(StmtDef *stmtdef);
            ~Obj() = default;

            /// @brief Get the identifier of this function
            const std::string &id(void) const;

            /// @brief Get the Statement Definition of this function
            StmtBlock *block(void) const;

            /// @brief Used to load this function's parameters
            /// with the values that were passed to it
            /// @param values The values to transfer
            void load_parameters(std::vector<value::Obj *> values);

            /// @brief Push this function's local scope
            void push_scope(void);

            /// @brief Pop this function's local scope
            void pop_scope(void);

            /// @brief Push a new scope context. This is needed
            /// for situations such as recursion
            void new_scope_context(void);

            /// @brief Remove a scope context. This happens in
            /// situations such as coming out of a recursive chain
            void drop_scope_context(void);

            /// @brief Check if a variable is in the local scope
            /// @param id The identifier of the variable
            bool has_local(const std::string &id);

            /// @brief Add a variable to this function's local scope
            /// @param var The the variable to add
            void add_local(variable::Obj *var);

            /// @brief Remove a variable from this function's local scope
            /// @param id The identifier of the variable to remove
            void remove_local(const std::string &id);

            /// @brief Get a variable in this funcion's local scope
            /// @note It is expected to call `has_local()` prior
            /// to calling this function
            /// @param id The identifier of the variable to get
            variable::Obj *get_local(const std::string &id);

            /// @brief Get this function's current context size
            size_t context_size(void);

            /// @brief Check if this function is a @world function.
            /// See `src/include/common.hpp/FuncAttrs`
            bool is_world(void) const;

            /// @note This local scope needs to be public for the context to see
            /// for a QAD solution.
            std::vector<Scope<std::string, variable::Obj *>> m_local;
        private:
            StmtDef *m_stmtdef;
        };
    };
};

#endif // EARL_H