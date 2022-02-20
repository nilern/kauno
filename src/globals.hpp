#ifndef GLOBALS_H
#define GLOBALS_H

#include <cstddef>

#include "object.hpp"
#include "symbol.hpp"

struct Var {
    ORef<Any> value;
};

static inline Handle<Var> Var_new(State* state, Handle<Any> value);

struct Globals {
    size_t count;
    size_t capacity;
    Symbol const** keys;
    Var** values;
};

static inline Globals Globals_new();

static inline void Globals_delete(Globals* globals);

static inline Var* Globals_find(Globals const* globals, ORef<Symbol> name);

static inline void Globals_insert(Globals* globals, ORef<Symbol> name, ORef<Var> var);

#endif // GLOBALS_H
