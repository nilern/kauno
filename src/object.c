#include "object.h"

#include <stdlib.h>
#include <string.h>

#include "gc.h"
#include "state.h"

static inline ORef oref_from_ptr(void* obj) { return (ORef){obj}; }

struct Header* obj_header(ORef obj) { return (struct Header*)obj.ptr - 1; }

void* obj_data(ORef obj) { return obj.ptr; }

static inline bool obj_is_marked(ORef obj) { return obj_header(obj)->bits & 1; }

static inline void obj_set_marked(ORef obj) { obj_header(obj)->bits |= 1; }

static inline struct Type* obj_type(ORef obj) {
    return (struct Type*)(void*)(obj_header(obj)->bits & ~1);
}

static inline void obj_set_type(ORef obj, ORef type) {
    obj_header(obj)->bits |= (size_t)type.ptr;
}

static inline ORef obj_field(struct State* state, ORef obj, size_t index) {
    struct Type* const type = obj_type(obj);

    if (index >= type->fields_count) {
        exit(EXIT_FAILURE); // FIXME
    }
    struct Field const field = type->fields[index];
    struct Type* const field_type = (struct Type*)obj_data(field.type);

    if (field_type->inlineable) {
        void* const field_obj = alloc(&state->heap, field_type);
        memcpy(field_obj, (void*)((char*)obj_data(obj) + field.offset), field_type->min_size);
        return oref_from_ptr(field_obj);
    } else {
        return *(ORef*)((char*)obj_data(obj) + field.offset);
    }
}

static inline bool obj_eq(ORef obj1, ORef obj2) { return obj1.ptr == obj2.ptr; }
