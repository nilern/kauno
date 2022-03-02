#ifndef FN_HPP
#define FN_HPP

#include "object.hpp"

class State;

namespace kauno::fn {

typedef Handle<void> (*CodePtr)(State* state);

struct Fn {
    CodePtr code;
    size_t domain_count;
    ORef<void> domain[0]; // ORef<Type | NoneType>
};

}

#endif // FN_HPP
