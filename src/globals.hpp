#ifndef GLOBALS_H
#define GLOBALS_H

#include <cstddef>

#include "object.hpp"

struct Var {
    ORef value;
};

static inline Handle Var_new(State* state, Handle value);

struct Globals {
    size_t count;
    size_t capacity;
    Symbol const** keys;
    Var** values;
};

static inline Globals Globals_new();

static inline void Globals_delete(Globals* globals);

static inline Var* Globals_find(Globals const* globals, Symbol const* name);

static inline void Globals_insert(Globals* globals, Symbol const* name, Var* var);

#endif // GLOBALS_H
