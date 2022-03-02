#ifndef AST_HPP
#define AST_HPP

#include "object.hpp"

struct Call {
    ORef<void> callee;
    size_t args_count;
    ORef<void> args[0];
};

#endif // AST_HPP
