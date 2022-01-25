#include "object.h"

static inline struct Object* deref(ORef obj) { return obj.ptr - 1; }

static inline bool obj_is_marked(ORef obj) { return deref(obj)->header.bits & 1; }

static inline void obj_set_marked(ORef obj) { deref(obj)->header.bits |= 1; }

static inline struct Type* obj_type(ORef obj) {
    return (struct Type*)(void*)(deref(obj)->header.bits & ~1);
}
