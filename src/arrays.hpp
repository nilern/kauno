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

}

#endif // ARRAYS_HPP
