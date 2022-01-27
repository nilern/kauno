#include "object.h"

ORef oref_from_ptr(void* obj) { return (ORef){obj}; }

struct Header* obj_header(ORef obj) { return (struct Header*)obj.ptr - 1; }

void* obj_data(ORef obj) { return obj.ptr; }

static inline bool obj_is_marked(ORef obj) { return obj_header(obj)->bits & 1; }

static inline void obj_set_marked(ORef obj) { obj_header(obj)->bits |= 1; }

static inline struct Type* obj_type(ORef obj) {
    return (struct Type*)(void*)(obj_header(obj)->bits & ~1);
}

void obj_set_type(ORef obj, ORef type) {
    obj_header(obj)->bits |= (size_t)type.ptr;
}

bool obj_eq(ORef obj1, ORef obj2) { return obj1.ptr == obj2.ptr; }
