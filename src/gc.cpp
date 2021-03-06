#include "gc.hpp"

#include <cstddef>
#include <cstdalign>
#include <cstring>
#include <algorithm>

#include "object.hpp"
#include "state.hpp"

namespace kauno::gc {

Granule const ALIGNMENT_HOLE = {0};

static inline bool granule_eq(Granule g1, Granule g2) { return g1.bits == g2.bits; }

void* Heap::alloc(Type* type) {
    // Compute alignment:
    size_t const align = type->align > alignof(Header) ? type->align : alignof(Header);

    // Compute obj start:
    size_t const free_ = (size_t)(void*)free;
    size_t data_start = free_ - type->min_size; // TODO: overflow check
    data_start &= ~(align - 1); // Align
    void* const obj = (void*)data_start;
    Header* const header = (Header*)data_start - 1; // Room for header

    if ((char*)header >= copied) {
        memset((void*)header, 0, free_ - (size_t)header); // Zero object, also adding alignment hole if needed
        ORef(obj).set_type(ORef(type)); // Initialize header

        free = (char*)header; // Bump end of free space

        return obj;
    } else {
        return NULL; // Out of fromspace
    }
}

void* Heap::alloc_indexed(Type* type, size_t indexed_count) {
    // TODO: Sanity checks:
    optional<ORef<Type>> const elem_type = type->fields[type->fields_count - 1].type.data();
    size_t elem_align = alignof(ORef<void>);
    size_t elem_size = sizeof(ORef<void>);
    if (elem_type.has_value()) {
        elem_align = elem_type->data()->align > alignof(size_t) ? elem_type->data()->align : alignof(size_t);
        elem_size = elem_type->data()->min_size;
    }

    // Compute alignment:
    size_t const align = type->align > alignof(Header) ? type->align : alignof(Header);

    // Compute obj start:
    size_t const free_ = (size_t)(void*)free;
    size_t indexed_start = free_ - indexed_count * elem_size; // TODO: overflow check
    indexed_start &= ~(elem_align - 1); // Align
    size_t data_start = indexed_start - type->min_size; // TODO: overflow check
    data_start &= ~(align - 1); // Align
    void* const obj = (void*)data_start;
    Header* const header = (Header*)data_start - 1; // Room for header

    if ((char*)header >= copied) {
        memset((void*)header, 0, free_ - (size_t)header); // Zero object, also adding alignment hole if needed
        ORef(obj).set_type(ORef(type)); // Initialize header
        *((size_t*)(void*)indexed_start - 1) = indexed_count; // Initialize indexed count

        free = (char*)header; // Bump end of free space

        return obj;
    } else {
        return NULL; // Out of fromspace
    }
}

template<typename T>
static inline ORef<T> mark(Heap*, ORef<T> obj) {
    // FIXME: copy object and set fwd ptr
    obj.set_marked();
    return obj;
}

static inline char* scan_field(Heap* heap, NRef<Type> field_type, char* data);

// Returns the address immediately after the object data.
static inline char* scan_fields(Heap* heap, Type* type, char* data) {
    if (!type->is_bits) {
        char* scan = data;

        if (!type->has_indexed) {
            for (size_t i = 0; i < type->fields_count; ++i) {
                scan = scan_field(heap, type->fields[i].type, data + type->fields[i].offset);
            }
        } else {
            size_t lasti = type->fields_count - 1;

            // Non-indexed fields:
            for (size_t i = 0; i < lasti; ++i) {
                scan_field(heap, type->fields[i].type, data + type->fields[i].offset);
            }

            // Elements of indexed field:
            NRef<Type> const indexed_type = type->fields[lasti].type;
            size_t const indexed_elem_size = indexed_type.has_value() ?
                        indexed_type.ptr()->min_size
                      : sizeof(ORef<void>);
            size_t* const indexed_data = (size_t*)(data + type->fields[lasti].offset);
            size_t const indexed_count = *indexed_data;
            char* indexed_fields = (char*)(indexed_data + 1);
            for (size_t i = 0; i < indexed_count; ++i) {
                scan = scan_field(heap, indexed_type, indexed_fields + indexed_elem_size * i);
            }
        }

        return scan;
    } else {
        // Pointer-free, just skip contents:
        return data + type->min_size;
    }
}

// Returns the address immediately after the field data.
static inline char* scan_field(Heap* heap, NRef<Type> field_type, char* field) {
    optional<ORef<Type>> const type = field_type.data();
    if (type.has_value() && type->data()->inlineable) {
        // Scan inlined fields:
        return scan_fields(heap, type->data(), field);
    } else {
        // Mark field:
        ORef<void>* const oref = (ORef<void>*)field;
        *oref = mark(heap, *oref);
        return (char*)(oref + 1);
    }
}

void Heap::collect() {
    // TODO: Mark roots

    // Copy live objects:
    char* scan = (char*)tospace.start;
    copied = (char*)tospace.start;
    while (scan < copied) {
        if (!granule_eq(*(Granule*)scan, ALIGNMENT_HOLE)) {
            // Scan object:
            ORef<void> const oref = ORef((void*)scan);
            oref.set_type(mark(this, ORef(oref.type()))); // mark type
            uintptr_t const addr = (uintptr_t)(void*)scan_fields(this, oref.type().data(), (char*)oref.data()); // scan fields
            scan = (char*)(void*)((addr + alignof(Granule) - 1) & ~(alignof(Granule) - 1));
        } else {
            // Skip alignment hole:
            ++scan;
        }
    }

    // Swap semispaces:
    std::swap(fromspace, tospace);
    free = (char*)fromspace.end;
}

}
