#ifndef TYPESMAP_HPP
#define TYPESMAP_HPP

#include <cassert>

#include "arrays.hpp"

template<typename T>
using RefArray = kauno::arrays::RefArray<T>;
using Type = kauno::Type;

namespace kauno {

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

    static KeyState probe_at(size_t arity, size_t count, ORef<Type> const* keys, ORef<Type> const* types) {
        if (arity > 0) {
            for (size_t i = 0; i < arity; ++i, ++keys, ++types) {
                if (types->data()) {
                    if (*keys != *types) {
                        return INEQUAL;
                    }
                } else {
                    return ABSENT;
                }

            }

            return EQUAL;
        } else {
            return count > 0 ? EQUAL : ABSENT;
        }
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
                    ORef<Type>* key = &new_keys.data()->elements[arity_ * j];

                    if (probe_at(arity_, count_, key, types) == ABSENT) {
                        memcpy(key, types, sizeof(ORef<Type>) * arity_);
                        new_values.data()->elements[j] = value;
                        break;
                    }
                    // else INEQUAL, EQUAL is impossible when rehashing
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
            ORef<Type> const* key = &keys_.data()->elements[arity_ * i];

            switch (probe_at(arity_, count_, key, types)) {
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
            ORef<Type>* key = &keys_.data()->elements[arity_ * i];

            switch (probe_at(arity_, count_, key, types)) {
            case ABSENT: {
                if ((count_ + 1) * 2 > cap) { // New load factor > 0.5
                    rehash(state);
                    goto recur; // `i` & `ki` were invalidated by rehash
                }
                ++count_;
                // Intentional fallthrough:
            }
            case EQUAL: {
                memcpy(key, types, sizeof(ORef<Type>) * arity_);
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
