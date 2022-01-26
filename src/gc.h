#ifndef GC_H
#define GC_H

#include <stdint.h>

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
    Granule* copied;
    Granule* free;
};

#endif // GC_H
