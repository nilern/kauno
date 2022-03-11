#ifndef ARRAYS_HPP
#define ARRAYS_HPP

#include <cstddef>

#include "state.hpp"

namespace kauno::arrays {

template<typename T>
struct RefArray {
    size_t count;
    ORef<T> elements[0];

    static ORef<RefArray<T>> create(State& state, size_t count) {
        return ORef(static_cast<RefArray<T>*>(state.alloc_indexed(state.RefArray.data(), count)));
    }
};

template<typename T>
struct NRefArray {
    size_t count;
    NRef<T> elements[0];

    static ORef<NRefArray<T>> create(State& state, size_t count) {
        return ORef(static_cast<NRefArray<T>*>(state.alloc_indexed(state.NRefArray.data(), count)));
    }
};

}

#endif // ARRAYS_HPP
