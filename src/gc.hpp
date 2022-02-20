#ifndef GC_H
#define GC_H

#include <cstdint>

#include "object.hpp"

struct State;

struct Granule {
    uintptr_t bits;
};

struct Semispace {
    Granule* start;
    Granule* end;
};

struct Heap {
    Semispace fromspace;
    Semispace tospace;
    char* copied;
    char* free;
};

static inline Heap Heap_new(size_t heap_size);
static inline void Heap_delete(Heap* heap);

// FIXME: Failing allocations should trigger collection:

static inline void* alloc(Heap* heap, Type* type);
static inline void* alloc_indexed(Heap* heap, Type* type, size_t indexed_count);

static inline void collect(State* state);

#endif // GC_H
