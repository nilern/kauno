#ifndef GC_H
#define GC_H

#include <stdint.h>

#include "state.h"

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

static inline struct Object* alloc(struct Heap* heap, ORef type);

// TODO: alloc_indexed

static inline void collect(struct State* state);

#endif // GC_H
