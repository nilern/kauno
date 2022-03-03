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
};

struct Fn {
    ORef<kauno::arrays::RefArray<void>> domain; // RefArray<Type | NoneType>
    ORef<void> body;
    size_t arity;
    ORef<Symbol> params[0];

    static bool const IS_BITS = false;
    static size_t const FIELDS_COUNT = 3;
    static bool const HAS_INDEXED = true;
    static bool const INLINEABLE = false;
};

}

#endif // AST_HPP
