#ifndef FN_HPP
#define FN_HPP

#include "object.hpp"

class State;

namespace kauno::fn {

typedef Handle<Any> (*CodePtr)(State* state);

struct Fn {
    CodePtr code;
};

}

#endif // FN_HPP
