#ifndef GC_H
#define GC_H

#include <stdint.h>

#include "object.h"

struct State;

typedef struct {
    uintptr_t bits;
} Granule;

struct Semispace {
    Granule* start;
    Granule* end;
};

struct Heap {
    struct Semispace fromspace;
    struct Semispace tospace;
    char* copied;
    char* free;
};

static inline struct Heap Heap_new(size_t heap_size);
static inline void Heap_delete(struct Heap* heap);

// FIXME: Failing allocations should trigger collection:

static inline void* alloc(struct Heap* heap, struct Type* type);
static inline void* alloc_indexed(struct Heap* heap, struct Type* type, size_t indexed_count);

static inline void collect(struct State* state);

#endif // GC_H
