#ifndef AST_HPP
#define AST_HPP

#include "object.hpp"

struct Call {
    ORef<Any> callee;
    size_t args_count;
    ORef<Any> args[0];
};

#endif // AST_HPP
