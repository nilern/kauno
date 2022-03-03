#ifndef ARRAYS_HPP
#define ARRAYS_HPP

#include <cstddef>

#include "object.hpp"

namespace kauno::arrays {

template<typename T>
struct RefArray {
    size_t count;
    ORef<T> elements[0];
};

}

#endif // ARRAYS_HPP
