#ifndef LOCALS_HPP
#define LOCALS_HPP

#include <cstddef>
#include <optional>

using std::optional;

#include "arrays.hpp"
#include "symbol.hpp"
#include "state.hpp"

namespace kauno {

using arrays::RefArray;
using arrays::NRefArray;

struct Locals {
    ORef<void> parent; // ORef<Locals | NoneType>
    ORef<NRefArray<void>> values;
    size_t capacity;
    NRef<Symbol> keys[0];

    static ORef<Type> create_reified(State& state) {
        size_t const fields_count = 3;

        Type* type = static_cast<Type*>(state.alloc_indexed(state.Type.data(), fields_count));
        *type = Type::create_indexed(state, alignof(struct Locals), sizeof(struct Locals), fields_count);
        type->fields[0] = (struct Field){NRef<struct Type>(), offsetof(struct Locals, parent)};
        type->fields[1] = (struct Field){NRef(state.RefArray), offsetof(struct Locals, values)};
        type->fields[2] = (struct Field){NRef(state.Symbol), offsetof(struct Locals, keys)};

        return ORef(type);
    }

    static ORef<Type> reify(State const& state) { return state.Locals; }

    static Handle<Locals> create(State& state, Handle<void> parent, size_t count) {
        size_t capacity = 2*count; // Load factor = 0.5

        ORef<NRefArray<void>> const values = NRefArray<void>::create(state, capacity);

        Locals* const locals = static_cast<Locals*>(state.alloc_indexed(state.Locals.data(), capacity));
        *locals = (Locals){
            .parent = parent.oref(),
            .values = values,
            .capacity = capacity,
            .keys = {}
        };

        return state.push_outlined(ORef(locals));
    }

    // FIXME: Error on duplicates
    void insert(ORef<Symbol> key, ORef<void> value) {
        size_t const hash = key.data()->hash;

        size_t const max_index = capacity - 1;
        for (size_t collisions = 0, i = hash & max_index;; ++collisions, i = (i + collisions) & max_index) {
            if (!keys[i].has_value()) {
                keys[i] = NRef(key);
                values.data()->elements[i] = NRef(value);
                return;
            }
        }
    }

    optional<ORef<void>> find(State& state, ORef<Symbol> key) const {
        size_t const hash = key.data()->hash;

        size_t const max_index = capacity - 1;
        for (size_t collisions = 0, i = hash & max_index;; ++collisions, i = (i + collisions) & max_index) {
            optional<ORef<Symbol>> const k = keys[i].data();

            if (k.has_value()) {
                if (k == key) { return optional(ORef(values.data()->elements[i].ptr())); }
            } else {
                return parent.is_instance<Locals>(state) ?
                            parent.unchecked_cast<Locals>().data()->find(state, key)
                          : optional<ORef<void>>();
            }
        }
    }
};

}

#endif // LOCALS_HPP
