#ifndef ARRAYS_HPP
#define ARRAYS_HPP

#include <cstddef>

#include "state.hpp"

namespace kauno::arrays {

template<typename T>
struct RefArray {
    size_t count;
    ORef<T> elements[0];

    static Handle<RefArray<T>> create(State& state, size_t count) {
        RefArray<T>* const array = static_cast<RefArray<T>*>(state.alloc_indexed(state.RefArray.data(), count));
        return state.push(ORef(array));
    }
};

}

#endif // ARRAYS_HPP
