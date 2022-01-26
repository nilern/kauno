#include "gc.h"

#include <stddef.h>
#include <stdalign.h>

#include "object.h"
#include "state.h"

Granule const ALIGNMENT_HOLE = {0};

bool granule_eq(Granule g1, Granule g2) { return g1.bits == g2.bits; }

static inline ORef mark(struct Heap* heap, ORef obj) {
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
            size_t const indexed_elem_size = ((struct Type*)deref(indexed_type))->min_size;
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
    struct Type* const type = (struct Type*)deref(field_type);
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
    Granule* scan = state->heap.tospace.start;
    state->heap.copied = state->heap.tospace.start;
    while (scan < state->heap.copied) {
        if (!granule_eq(*scan, ALIGNMENT_HOLE)) {
            // Scan object:
            ORef const obj = oref_from_ptr((struct Object*)scan);
            uintptr_t const addr = (uintptr_t)(void*)scan_fields(state, obj_type(obj), obj_data(obj));
            scan = (Granule*)(void*)((addr + alignof(Granule) - 1) & ~(alignof(Granule) - 1));
        } else {
            // Skip alignment hole:
            ++scan;
        }
    }

    // Swap semispaces:
    struct Semispace tmp = state->heap.fromspace;
    state->heap.fromspace = state->heap.tospace;
    state->heap.tospace = tmp;
    state->heap.free = state->heap.tospace.end;
}
