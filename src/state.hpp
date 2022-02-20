#ifndef STATE_H
#define STATE_H

#include <cstdio>

#include "gc.hpp"
#include "symbol.hpp"
#include "globals.hpp"

struct State {
    Heap heap;

    struct Type* Type;
    struct Type* UInt8;
    struct Type* Int64;
    struct Type* USize;
    struct Type* Bool;
    struct Type* Symbol;
    struct Type* Any;
   struct  Type* Var;

    SymbolTable symbols;

    Globals globals;

    ORef* sp;
    size_t stack_size;
    ORef* stack; // TODO: Growable ("infinite") stack
};

static inline State State_new(size_t heap_size, size_t stack_size);

static inline void State_delete(State* state);

static inline Handle State_push(State* state, ORef value);

static inline Handle State_peek(State* state);

static inline void State_pop(State* state);

static inline void State_popn(State* state, size_t n);

static inline void State_print_builtin(State const* state, FILE* dest, Handle value);

#endif // STATE_H
