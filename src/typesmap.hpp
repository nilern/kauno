#ifndef TYPESMAP_HPP
#define TYPESMAP_HPP

#include "arrays.hpp"

template<typename T>
using RefArray = kauno::arrays::RefArray<T>;
using Type = kauno::Type;

namespace kauno {

// FIXME: Probably breaks for zero arity:
class TypesMap {
    size_t arity_;
    size_t count_;
    ORef<RefArray<Type>> keys_;
    ORef<RefArray<void>> values_;

    enum KeyState {EQUAL, INEQUAL, ABSENT};

    size_t capacity() const { return values_.data()->count; }

    size_t hash_types(ORef<Type> const* const types) const {
        size_t hash = 0;

        for (size_t i = 0; i < arity_; ++i) {
            hash = 31 * hash + types[i].data()->hash;
        }

        return hash;
    }

    KeyState probe_at(size_t ki, ORef<Type> const* types) const {
        size_t const limit = ki + arity_;
        for (; ki < limit; ++ki, ++types) {
            if (types->data()) {
                if (keys_.data()->elements[ki] != *types) {
                    return INEQUAL;
                }
            } else {
                return ABSENT;
            }

        }

        return EQUAL;
    }

    void rehash(State& state) {
        size_t const cap = capacity();
        size_t const new_capacity = cap * 2;

        ORef<RefArray<Type>> const new_keys = RefArray<Type>::create(state, arity_ * new_capacity);
        ORef<RefArray<void>> const new_values = RefArray<void>::create(state, new_capacity);

        for (size_t i = 0; i < cap; ++i) {
            size_t const ki = arity_ * i;

            ORef<void> const value = values_.data()->elements[i];
            if (value.data()) {
                ORef<Type> const* const types = &keys_.data()->elements[ki];
                size_t const hash = hash_types(types);

                size_t const max_index = new_capacity - 1;
                for (size_t collisions = 0, j = hash & max_index;; ++collisions, j = (j + collisions) & max_index) {
                    size_t const kj = arity_ * j;

                    if (!new_keys.data()->elements[kj].data()) {
                        memcpy(&new_keys.data()->elements[kj], types, sizeof(ORef<Type>) * arity_);
                        new_values.data()->elements[j] = value;
                    }
                }
            }
        }

        keys_ = new_keys;
        values_ = new_values;
    }

public:
    static ORef<Type> create_reified(State& state) {
        size_t const fields_count = 4;

        Type* type = static_cast<Type*>(state.alloc_indexed(state.Type.data(), fields_count));
        *type = Type::create_record(state, alignof(TypesMap), sizeof(TypesMap), false /* `count_` is mutable */, fields_count);
        type->fields[0] = Field(state.USize, offsetof(TypesMap, arity_));
        type->fields[1] = Field(state.USize, offsetof(TypesMap, count_));
        type->fields[2] = Field(state.RefArray, offsetof(TypesMap, keys_));
        type->fields[3] = Field(state.RefArray, offsetof(TypesMap, values_));

        return ORef(type);
    }

    ORef<Type> reify(State const& state) { return state.TypesMap; }

    TypesMap(State& state, size_t arity) :
        arity_(arity),
        count_(0),
        keys_(RefArray<Type>::create(state, arity * 2)),
        values_(RefArray<void>::create(state, 2))
    {}

    optional<ORef<void>> get(ORef<Type> const* types) const {
        size_t const hash = hash_types(types);

        size_t const max_index = capacity() - 1;
        for (size_t collisions = 0, i = hash & max_index;; ++collisions, i = (i + collisions) & max_index) {
            size_t const ki = arity_ * i;

            switch (probe_at(ki, types)) {
            case EQUAL: return optional(values_.data()->elements[i]);
            case ABSENT: return optional<ORef<void>>();
            case INEQUAL: {}
            }
        }
    }

    void set(State& state, ORef<Type> const* types, Handle<void> value) {
        size_t const hash = hash_types(types);

    recur:
        size_t const cap = capacity();
        size_t const max_index = cap - 1;
        for (size_t collisions = 0, i = hash & max_index;; ++collisions, i = (i + collisions) & max_index) {
            size_t const ki = arity_ * i;

            switch (probe_at(ki, types)) {
            case ABSENT: {
                if ((count_ + 1) * 2 > cap) { // New load factor > 0.5
                    rehash(state);
                    goto recur; // `i` & `ki` were invalidated by rehash
                }
                // Else fall through to:
            }
            case EQUAL: {
                ++count_;
                memcpy(&keys_.data()->elements[ki], types, sizeof(ORef<Type>) * arity_);
                values_.data()->elements[i] = value.oref();
                return;
            }
            case INEQUAL: {}
            }
        }
    }
};

}

#endif // TYPESMAP_HPP
