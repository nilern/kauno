#ifndef STATE_H
#define STATE_H

#include <stdio.h>

#include "gc.h"

#define STACK_SIZE 1024*1024

struct State {
    struct Heap heap;

    struct Type* Type;
    struct Type* Int64;
    struct Type* USize;
    struct Type* Bool;

    ORef* sp;
    ORef stack[STACK_SIZE / sizeof(ORef)]; // TODO: Growable ("infinite") stack
};

static inline struct State State_new(size_t heap_size);

static inline void State_push(struct State* state, ORef value);

static inline ORef State_pop(struct State* state);

static inline void State_print_builtin(struct State const* state, FILE* dest, ORef value);

#endif // STATE_H
