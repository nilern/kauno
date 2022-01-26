#include "object.h"

ORef oref_from_ptr(struct Object *obj) { return (ORef){obj + 1}; }

static inline struct Object* deref(ORef obj) { return obj.ptr - 1; }

char *obj_data(ORef obj) { return (char*)obj.ptr; }

static inline bool obj_is_marked(ORef obj) { return deref(obj)->header.bits & 1; }

static inline void obj_set_marked(ORef obj) { deref(obj)->header.bits |= 1; }

static inline struct Type* obj_type(ORef obj) {
    return (struct Type*)(void*)((deref(obj)->header.bits & ~1) - 1);
}

bool obj_eq(ORef obj1, ORef obj2) { return obj1.ptr == obj2.ptr; }
