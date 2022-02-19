#ifndef GLOBALS_H
#define GLOBALS_H

#include <stddef.h>

#include "object.h"

struct Var {
    ORef value;
};

static inline Handle Var_new(struct State* state, Handle value);

struct Globals {
    size_t count;
    size_t capacity;
    struct Symbol const** keys;
    struct Var** values;
};

static inline struct Globals Globals_new();

static inline void Globals_delete(struct Globals* globals);

static inline struct Var* Globals_find(struct Globals const* globals, struct Symbol const* name);

static inline void Globals_insert(struct Globals* globals, struct Symbol const* name, struct Var* var);

#endif // GLOBALS_H
