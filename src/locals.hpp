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

struct Locals {
    ORef<void> parent; // ORef<Locals | NoneType>
    ORef<RefArray<void>> values;
    size_t capacity;
    ORef<Symbol> keys[0];

    static bool const IS_BITS = false;
    static size_t const FIELDS_COUNT = 3;
    static bool const HAS_INDEXED = true;
    static bool const INLINEABLE = false;

    static Handle<Locals> create(State& state, Handle<void> parent, size_t count) {
        size_t capacity = 2*count; // Load factor = 0.5

        Handle<RefArray<void>> const values = RefArray<void>::create(state, capacity);

        Locals* const locals = static_cast<Locals*>(state.alloc_indexed(state.Locals.data(), capacity));
        *locals = (Locals){
            .parent = parent.oref(),
            .values = values.oref(),
            .capacity = capacity,
            .keys = {}
        };

        state.pop(); // `values`
        return state.push(ORef(locals));
    }

    void insert(ORef<Symbol> key, ORef<void> value) {
        size_t const hash = key.data()->hash;

        size_t const max_index = capacity - 1;
        for (size_t collisions = 0, i = hash & max_index;; ++collisions, i = (i + collisions) & max_index) {
            if (!keys[i].data()) {
                keys[i] = key;
                values.data()->elements[i] = value;
                return;
            }
        }
    }

    optional<ORef<void>> find(State& state, ORef<Symbol> key) const {
        size_t const hash = key.data()->hash;

        size_t const max_index = capacity - 1;
        for (size_t collisions = 0, i = hash & max_index;; ++collisions, i = (i + collisions) & max_index) {
            ORef<Symbol> const k = keys[i];

            if (k == key) { return optional(values.data()->elements[i]); }

            if (!k.data()) {
                return parent.is_instance(state.Locals) ?
                            parent.unchecked_cast<Locals>().data()->find(state, key)
                          : optional<ORef<void>>();
            }
        }
    }
};

}

#endif // LOCALS_HPP
