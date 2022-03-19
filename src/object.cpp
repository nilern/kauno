#include "object.hpp"

#include "state.hpp"

namespace kauno {

void DynRef::repush(State& state) const {
    optional<ORef<Type>> const opt_type = type_.data();
    if (opt_type.has_value()) {
        ORef<struct Type> const type = *opt_type;
        SRef<void> const sref = state.stack_alloc<void>(type);
        memmove(sref.data(), data(), type.data()->min_size);
    } else {
        state.push_outlined(unchecked_oref());
    }
}

ORef<void> DynRef::to_heap(State& state) const {
    optional<ORef<Type>> const opt_type = type_.data();
    if (opt_type.has_value()) {
        Type* const type = opt_type->data();
        size_t const size = type->min_size;

        void* const data = state.alloc(type);
        memcpy(data, sptr_, size);

        return ORef(data);
    } else {
        return unchecked_oref();
    }
}

ORef<Type> Type::reify(State const& state) { return state.Type; }

Type::Type(State& state,
           size_t align_, size_t min_size_, bool inlineable_, bool is_bits_, bool has_indexed_, size_t fields_count_) :
    hash(state.create_type_hash()),
    align(align_), min_size(min_size_), inlineable(inlineable_), is_bits(is_bits_), has_indexed(has_indexed_), fields_count(fields_count_),
    fields{}
{}

}
