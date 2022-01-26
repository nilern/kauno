#ifndef OBJECT_H
#define OBJECT_H

#include <stdbool.h>
#include <stddef.h>
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

struct Field {
    size_t offset;
    ORef type;
};

struct Type {
    struct Object base;
    size_t align;
    size_t min_size;
    bool inlineable;
    bool is_bits;
    bool has_indexed;
    size_t fields_count; // if is_bits then byte count else field_types count
    struct Field fields[];
};

static inline ORef oref_from_ptr(struct Object* obj);

static inline struct Object* deref(ORef obj);

static inline char* obj_data(ORef obj);

static inline bool obj_is_marked(ORef obj);

static inline void obj_set_marked(ORef obj);

static inline struct Type* obj_type(ORef obj);

static inline bool obj_eq(ORef obj1, ORef obj2);

#endif // OBJECT_H
