#include "object.hpp"

#include "state.hpp"

namespace kauno {

ORef<Type> Type::reify(State const& state) { return state.Type; }

}
