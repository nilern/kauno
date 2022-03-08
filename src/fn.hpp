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

    static ORef<Type> reify(State const& state) { return state.Fn; }
};

struct Closure {
    ORef<kauno::ast::Fn> code;
    ORef<void> env; // ORef<Locals | NoneType>

    static ORef<Type> create_reified(State& state) {
        size_t const fields_count = 2;

        Type* type = static_cast<struct Type*>(state.alloc_indexed(state.Type.data(), fields_count));
        *type = Type::create_record(state, alignof(Closure), sizeof(Closure), true, fields_count);
        type->fields[0] = (struct Field){state.AstFn, offsetof(Closure, code)};
        type->fields[1] = (struct Field){ORef<struct Type>(nullptr), offsetof(Closure, env)};

        return ORef(type);
    }

    static ORef<Type> reify(State const& state) { return state.Closure; }

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
