#include "object.h"

ORef oref_from_ptr(struct Object *obj) { return (ORef){obj + 1}; }

static inline struct Object* deref(ORef obj) { return obj.ptr - 1; }

char *obj_data(ORef obj) { return (char*)obj.ptr; }

static inline bool obj_is_marked(ORef obj) { return deref(obj)->header.bits & 1; }

static inline void obj_set_marked(ORef obj) { deref(obj)->header.bits |= 1; }

static inline struct Type* obj_type(ORef obj) {
    return (struct Type*)((struct Object*)(void*)(deref(obj)->header.bits & ~1) - 1);
}

void obj_set_type(ORef obj, ORef type) {
    deref(obj)->header.bits |= (size_t)type.ptr;
}

bool obj_eq(ORef obj1, ORef obj2) { return obj1.ptr == obj2.ptr; }
