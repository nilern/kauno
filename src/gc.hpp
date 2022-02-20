#ifndef GC_H
#define GC_H

#include <cstdint>

#include "object.hpp"

struct State;

struct Granule {
    uintptr_t bits;
};

class Heap {
    struct Semispace {
        Granule* start;
        Granule* end;

        Semispace(size_t size) {
            char* const mem = new char[size];
            start = (Granule*)mem;
            end = (Granule*)(mem + size);
        }

        Semispace(Semispace&& other) : start(other.start), end(other.end) {
            other.start = nullptr;
        }

        Semispace& operator=(Semispace&& other) {
            start = other.start;
            end = other.end;
            other.start = nullptr;
            return *this;
        }

        Semispace(Semispace const& semi) = delete;
        Semispace& operator=(Semispace const& semi) = delete;

        ~Semispace() {
            delete [] start;
        }
    };

    Semispace fromspace;
    Semispace tospace;
    char* copied;
    char* free;

public:
    // FIXME: Convert heap_size to granules properly:
    Heap(size_t heap_size) : fromspace(heap_size / 2), tospace(heap_size / 2) {
        copied = (char*)fromspace.start;
        free = (char*)fromspace.end;
    }

    // FIXME: Failing allocations should trigger collection:

    void* alloc(Type* type);
    void* alloc_indexed(Type* type, size_t indexed_count);

    void collect(State* state);
};

#endif // GC_H
