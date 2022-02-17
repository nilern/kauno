#include "gc.h"

#include <stddef.h>
#include <stdalign.h>
#include <string.h>
#include <stdlib.h>

#include "object.h"
#include "state.h"

Granule const ALIGNMENT_HOLE = {0};

static inline bool granule_eq(Granule g1, Granule g2) { return g1.bits == g2.bits; }

static inline struct Semispace Semispace_new(size_t size) {
    char* const mem = malloc(size);
    return (struct Semispace){
        .start = (Granule*)mem,
        .end = (Granule*)(mem + size)
    };
}

static inline void Semispace_delete(struct Semispace* semispace) {
    free(semispace->start);
}

// FIXME: Convert heap_size to granules properly:
static inline struct Heap Heap_new(size_t heap_size) {
    size_t const semi_size = heap_size / 2;
    struct Semispace const fromspace = Semispace_new(semi_size);
    struct Semispace const tospace = Semispace_new(semi_size);
    return (struct Heap){
        .fromspace = fromspace,
        .tospace = tospace,
        .copied = (char*)fromspace.start,
        .free = (char*)fromspace.end
    };
}

static inline void Heap_delete(struct Heap* heap) {
    Semispace_delete(&heap->fromspace);
    Semispace_delete(&heap->tospace);
}

static inline void* alloc(struct Heap* heap, struct Type* type) {
    // Compute alignment:
    size_t const align = type->align > alignof(struct Header) ? type->align : alignof(struct Header);

    // Compute obj start:
    size_t const free = (size_t)(void*)heap->free;
    size_t data_start = free - type->min_size; // TODO: overflow check
    data_start &= ~(align - 1); // Align
    void* const obj = (void*)data_start;
    struct Header* const header = (struct Header*)data_start - 1; // Room for header

    if ((char*)header >= heap->copied) {
        memset((void*)header, 0, free - (size_t)header); // Zero object, also adding alignment hole if needed
        obj_set_type(oref_from_ptr(obj), oref_from_ptr(type)); // Initialize header

        heap->free = (char*)header; // Bump end of free space

        return obj;
    } else {
        return NULL; // Out of fromspace
    }
}

static inline void* alloc_indexed(struct Heap* heap, struct Type* type, size_t indexed_count) {
    // TODO: Sanity checks:
    struct Type* const elem_type = (struct Type*)obj_data(type->fields[type->fields_count - 1].type);

    // Compute alignment:
    size_t const align = type->align > alignof(struct Header) ? type->align : alignof(struct Header);
    size_t const elem_align = elem_type->align > alignof(size_t) ? elem_type->align : alignof(size_t);

    // Compute obj start:
    size_t const free = (size_t)(void*)heap->free;
    size_t indexed_start = free - indexed_count * elem_type->min_size; // TODO: overflow check
    indexed_start &= ~(elem_align - 1); // Align
    size_t data_start = indexed_start - type->min_size; // TODO: overflow check
    data_start = data_start & ~(align - 1); // Align
    void* const obj = (void*)data_start;
    struct Header* const header = (struct Header*)data_start - 1; // Room for header

    if ((char*)header >= heap->copied) {
        memset((void*)header, 0, free - (size_t)header); // Zero object, also adding alignment hole if needed
        obj_set_type(oref_from_ptr(obj), oref_from_ptr(type)); // Initialize header
        *((size_t*)(void*)indexed_start - 1) = indexed_count; // Initialize indexed count

        heap->free = (char*)header; // Bump end of free space

        return obj;
    } else {
        return NULL; // Out of fromspace
    }
}

static inline ORef mark(struct Heap*, ORef obj) {
    // FIXME: copy object and set fwd ptr
    obj_set_marked(obj);
    return obj;
}

static inline char* scan_field(struct State* state, ORef field_type, char* data);

// Returns the address immediately after the object data.
static inline char* scan_fields(struct State* state, struct Type* type, char* data) {
    if (!type->is_bits) {
        char* scan = data;

        if (!type->has_indexed) {
            for (size_t i = 0; i < type->fields_count; ++i) {
                scan = scan_field(state, type->fields[i].type, data + type->fields[i].offset);
            }
        } else {
            size_t lasti = type->fields_count - 1;

            // Non-indexed fields:
            for (size_t i = 0; i < lasti; ++i) {
                scan_field(state, type->fields[i].type, data + type->fields[i].offset);
            }

            // Elements of indexed field:
            ORef const indexed_type = type->fields[lasti].type;
            size_t const indexed_elem_size = ((struct Type*)obj_data(indexed_type))->min_size;
            size_t* const indexed_data = (size_t*)(data + type->fields[lasti].offset);
            size_t const indexed_count = *indexed_data;
            char* indexed_fields = (char*)(indexed_data + 1);
            for (size_t i = 0; i < indexed_count; ++i) {
                scan = scan_field(state, indexed_type, indexed_fields + indexed_elem_size * i);
            }
        }

        return scan;
    } else {
        // Pointer-free, just skip contents:
        return data + type->min_size;
    }
}

// Returns the address immediately after the field data.
static inline char* scan_field(struct State* state, ORef field_type, char* field) {
    struct Type* const type = (struct Type*)obj_data(field_type);
    if (type->inlineable) {
        // Scan inlined fields:
        return scan_fields(state, type, field);
    } else {
        // Mark field:
        ORef* const oref = (ORef*)field;
        *oref = mark(&state->heap, *oref);
        return (char*)(oref + 1);
    }
}

static inline void collect(struct State* state) {
    // TODO: Mark roots

    // Copy live objects:
    char* scan = (char*)state->heap.tospace.start;
    state->heap.copied = (char*)state->heap.tospace.start;
    while (scan < state->heap.copied) {
        if (!granule_eq(*(Granule*)scan, ALIGNMENT_HOLE)) {
            // Scan object:
            ORef const oref = oref_from_ptr((void*)scan);
            obj_set_type(oref, mark(&state->heap, oref_from_ptr((void*)obj_type(oref)))); // mark type
            uintptr_t const addr = (uintptr_t)(void*)scan_fields(state, obj_type(oref), obj_data(oref)); // scan fields
            scan = (char*)(void*)((addr + alignof(Granule) - 1) & ~(alignof(Granule) - 1));
        } else {
            // Skip alignment hole:
            ++scan;
        }
    }

    // Swap semispaces:
    struct Semispace tmp = state->heap.fromspace;
    state->heap.fromspace = state->heap.tospace;
    state->heap.tospace = tmp;
    state->heap.free = (char*)state->heap.fromspace.end;
}
