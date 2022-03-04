#include "ast.hpp"

namespace kauno::ast {

ORef<Type> Call::reify(State const& state) { return state.Call; }

ORef<Type> Fn::reify(State const& state) { return state.AstFn; }

}
