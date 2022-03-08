#ifndef AST_HPP
#define AST_HPP

#include "object.hpp"
#include "symbol.hpp"

namespace kauno::arrays {
    template<typename T>
    struct RefArray;
}

namespace kauno::ast {

struct Call {
    ORef<void> callee;
    size_t args_count;
    ORef<void> args[0];

    static ORef<Type> reify(State const& state);
};

struct Fn {
    ORef<kauno::arrays::RefArray<void>> domain; // RefArray<Type | NoneType>
    ORef<void> body;
    size_t arity;
    ORef<Symbol> params[0];

    static ORef<Type> create_reified(State& state);

    static ORef<Type> reify(State const& state);
};

}

#endif // AST_HPP
