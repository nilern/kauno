#include "object.h"

#include <stdlib.h>
#include <string.h>

#include "gc.h"
#include "state.h"

static inline ORef oref_from_ptr(void* obj) { return (ORef){obj}; }

static inline ORef Handle_oref(Handle handle) { return *handle.oref_ptr; }

static inline struct Header* obj_header(ORef obj) { return (struct Header*)obj.ptr - 1; }

static inline void* obj_data(ORef obj) { return obj.ptr; }

static inline bool obj_is_marked(ORef obj) { return obj_header(obj)->bits & 1; }

static inline void obj_set_marked(ORef obj) { obj_header(obj)->bits |= 1; }

static inline struct Type* obj_type(ORef obj) {
    return (struct Type*)(void*)(obj_header(obj)->bits & ~1);
}

static inline void obj_set_type(ORef obj, ORef type) {
    obj_header(obj)->bits |= (size_t)type.ptr;
}

static inline ORef obj_field(struct State* state, Handle handle, size_t index) {
    ORef const obj = Handle_oref(handle);

    struct Type* const type = obj_type(obj);

    if (index >= type->fields_count) {
        exit(EXIT_FAILURE); // FIXME
    }
    struct Field const field = type->fields[index];
    struct Type* const field_type = (struct Type*)obj_data(field.type);

    if (field_type->inlineable) {
        void* const field_obj = alloc(&state->heap, field_type);
        struct Type* const field_type = (struct Type*)obj_data(obj_type(obj)->fields[index].type);
        memcpy(field_obj, (void*)((char*)obj_data(obj) + field.offset), field_type->min_size);
        return oref_from_ptr(field_obj);
    } else {
        return *(ORef*)((char*)obj_data(obj) + field.offset);
    }
}

static inline void obj_field_set(struct State*, Handle handle, size_t index, Handle new_val_handle) {
    ORef const obj = Handle_oref(handle);

    struct Type* const type = obj_type(obj);

    if (index >= type->fields_count) {
        exit(EXIT_FAILURE); // FIXME
    }
    struct Field const field = type->fields[index];

    ORef const new_val = Handle_oref(new_val_handle);

    // TODO: Polymorphic fields:
    if (!obj_eq(oref_from_ptr((void*)obj_type(new_val)), field.type)) {
        exit(EXIT_FAILURE); // FIXME
    }
    struct Type* const field_type = (struct Type*)obj_data(field.type);

    if (field_type->inlineable) {
        memcpy((void*)((char*)obj_data(obj) + field.offset), obj_data(new_val), field_type->min_size);
    } else {
        *(ORef*)((char*)obj_data(obj) + field.offset) = new_val;
    }
}

static inline bool obj_eq(ORef obj1, ORef obj2) { return obj1.ptr == obj2.ptr; }
