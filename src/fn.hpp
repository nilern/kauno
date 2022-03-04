#ifndef FN_HPP
#define FN_HPP

#include "object.hpp"
#include "ast.hpp"
#include "locals.hpp"

namespace kauno {
class State;
}

namespace kauno::fn {

typedef Handle<void> (*CodePtr)(State* state);

struct Fn {
    CodePtr code;
    size_t domain_count;
    ORef<void> domain[0]; // ORef<Type | NoneType>
};

struct Closure {
    ORef<kauno::ast::Fn> code;
    ORef<void> env; // ORef<Locals | NoneType>

    static bool const IS_BITS = false;
    static size_t const FIELDS_COUNT = 2;
    static bool const HAS_INDEXED = false;
    static bool const INLINEABLE = true;

    static Handle<Closure> create(State& state, Handle<kauno::ast::Fn> code, Handle<void> env) {
        Closure* closure = static_cast<Closure*>(state.alloc(state.Closure.data()));
        *closure = (Closure){
            .code = code.oref(),
            .env = env.oref()
        };
        return state.push(ORef(closure));
    }
};

}

#endif // FN_HPP
