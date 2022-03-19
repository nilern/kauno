#ifndef FN_HPP
#define FN_HPP

#include "object.hpp"
#include "ast.hpp"
#include "locals.hpp"

namespace kauno {
class State;
}

namespace kauno::fn {

typedef AnySRef (*CodePtr)(State& state);

struct Fn {
    CodePtr code;
    size_t domain_count;
    ORef<void> domain[0]; // ORef<Type | NoneType>

    static ORef<Type> reify(State const& state) { return state.Fn; }
};

struct Closure {
    ORef<kauno::ast::Fn> code;
    ORef<void> env; // ORef<Locals | NoneType>

    static ORef<Type> create_reified(State& state) {
        size_t const fields_count = 2;

        Type* type = static_cast<struct Type*>(state.alloc_indexed(state.Type.data(), fields_count));
        *type = Type::create_record(state, alignof(Closure), sizeof(Closure), true, fields_count);
        type->fields[0] = (struct Field){NRef(state.AstFn), offsetof(Closure, code)};
        type->fields[1] = (struct Field){NRef<struct Type>(), offsetof(Closure, env)};

        return ORef(type);
    }

    static ORef<Type> reify(State const& state) { return state.Closure; }

    static SRef<Closure> create(State& state, ORef<kauno::ast::Fn> code, ORef<void> env) {
        SRef<Closure> closure = state.stack_alloc<Closure>(state.Closure);
        *closure.data() = (Closure){
            .code = code,
            .env = env
        };
        return closure;
    }
};

}

#endif // FN_HPP
