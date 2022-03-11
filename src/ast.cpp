#include "ast.hpp"

namespace kauno::ast {

ORef<Type> Call::reify(State const& state) { return state.Call; }

ORef<Type> Fn::create_reified(State& state) {
    size_t const fields_count = 3;

    Type* type = static_cast<Type*>(state.alloc_indexed(state.Type.data(), fields_count));
    *type = Type::create_indexed(state, alignof(Fn), sizeof(Fn), fields_count);
    type->fields[0] = (struct Field){NRef(state.RefArray), offsetof(Fn, domain)};
    type->fields[1] = (struct Field){NRef<struct Type>(), offsetof(Fn, body)};
    type->fields[2] = (struct Field){NRef(state.Symbol), offsetof(Fn, params)};

    return ORef(type);
}

ORef<Type> Fn::reify(State const& state) { return state.AstFn; }

}
