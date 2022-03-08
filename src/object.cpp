#include "object.hpp"

#include "state.hpp"

namespace kauno {

ORef<Type> Type::reify(State const& state) { return state.Type; }

Type::Type(State& state,
           size_t align_, size_t min_size_, bool inlineable_, bool is_bits_, bool has_indexed_, size_t fields_count_) :
    hash(state.create_type_hash()),
    align(align_), min_size(min_size_), inlineable(inlineable_), is_bits(is_bits_), has_indexed(has_indexed_), fields_count(fields_count_),
    fields{}
{}

}
