#ifndef OBJECT_H
#define OBJECT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct State;

struct Header {
    uintptr_t bits;
};

typedef struct {
    void* ptr;
} ORef;

struct Field {
    size_t offset;
    ORef type;
};

struct Type {
    size_t align;
    size_t min_size;
    bool inlineable;
    bool is_bits;
    bool has_indexed;
    size_t fields_count; // if is_bits then byte count else field_types count
    struct Field fields[];
};

static inline ORef oref_from_ptr(void* obj);

static inline struct Header* obj_header(ORef obj);

static inline void* obj_data(ORef obj);

static inline bool obj_is_marked(ORef obj);

static inline void obj_set_marked(ORef obj);

static inline struct Type* obj_type(ORef obj);

static inline void obj_set_type(ORef obj, ORef type);

static inline ORef obj_field(struct State* state, ORef obj, size_t index);
// TODO: obj_field_indexed

static inline void obj_field_set(struct State* state, ORef obj, size_t index, ORef new_val);
// TODO: obj_field_indexed_set

static inline bool obj_eq(ORef obj1, ORef obj2);

#endif // OBJECT_H
