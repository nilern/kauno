#ifndef OBJECT_H
#define OBJECT_H

#include <stdbool.h>
#include <stdint.h>

struct Header {
    uintptr_t bits;
};

struct Object {
    struct Header header;
};

typedef struct {
    struct Object* ptr;
} ORef;

struct Type {
    struct Object base;
    bool is_bits;
    bool has_indexed;
    uintptr_t count; // if is_bits then byte count else field_types count
    struct Type* field_types[];
};

static inline bool obj_is_marked(ORef obj);

static inline void obj_set_marked(ORef obj);

static struct Type* obj_type(ORef obj);

#endif // OBJECT_H
