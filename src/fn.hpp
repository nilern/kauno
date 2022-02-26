#ifndef FN_HPP
#define FN_HPP

#include "object.hpp"

class State;

namespace kauno::fn {

typedef Handle<Any> (*CodePtr)(State* state);

struct Fn {
    CodePtr code;
    size_t domain_count;
    ORef<Type> domain[0];
};

}

#endif // FN_HPP
