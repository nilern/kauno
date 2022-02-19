#ifndef STATE_H
#define STATE_H

#include <stdio.h>

#include "gc.h"
#include "symbol.h"
#include "globals.h"

struct State {
    struct Heap heap;

    struct Type* Type;
    struct Type* UInt8;
    struct Type* Int64;
    struct Type* USize;
    struct Type* Bool;
    struct Type* Symbol;
    struct Type* Any;
    struct Type* Var;

    struct SymbolTable symbols;

    struct Globals globals;

    ORef* sp;
    size_t stack_size;
    ORef* stack; // TODO: Growable ("infinite") stack
};

static inline struct State State_new(size_t heap_size, size_t stack_size);

static inline void State_delete(struct State* state);

static inline Handle State_push(struct State* state, ORef value);

static inline Handle State_peek(struct State* state);

static inline void State_pop(struct State* state);

static inline void State_popn(struct State* state, size_t n);

static inline void State_print_builtin(struct State const* state, FILE* dest, Handle value);

#endif // STATE_H
