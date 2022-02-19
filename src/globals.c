#include "globals.h"

#include <stdlib.h>
#include <string.h>

#include "symbol.h"

static inline Handle Var_new(struct State* state, Handle value) {
    struct Var* var = (struct Var*)alloc(&state->heap, state->Var);
    *var = (struct Var){.value = Handle_oref(value)};
    return State_push(state, oref_from_ptr(var));
}

static inline struct Globals Globals_new() {
    size_t const capacity = 2;

    size_t const keys_size = sizeof(struct Symbol const*)*capacity;
    struct Symbol const** const keys = (struct Symbol const**)malloc(keys_size);
    memset(keys, 0, keys_size);

    size_t const values_size = sizeof(struct Var*)*capacity;
    struct Var** const values = (struct Var**)malloc(values_size);
    memset(values, 0, values_size);

    return (struct Globals){
        .count = 0,
        .capacity = capacity,
        .keys = keys,
        .values = values
    };
}

static inline void Globals_rehash(struct Globals* globals) {
    size_t const new_capacity = globals->capacity*2;

    size_t const keys_size = sizeof(struct Symbol const*)*new_capacity;
    struct Symbol const** const new_keys = (struct Symbol const**)malloc(keys_size);
    memset(new_keys, 0, keys_size);

    size_t const values_size = sizeof(struct Var*)*new_capacity;
    struct Var** const new_values = (struct Var**)malloc(values_size);
    memset(new_values, 0, values_size);

    for (size_t i = 0; i < globals->capacity; ++i) {
        struct Symbol const* const key = globals->keys[i];

        if (key) {
            size_t const hash = key->hash;

            size_t const max_index = new_capacity - 1;
            for (size_t collisions = 0, j = hash & max_index;; ++collisions, j = (j + collisions) & max_index) {
                if (!new_keys[j]) {
                    new_keys[j] = key;
                    new_values[j] = globals->values[i];
                    break;
                }
            }
        }
    }

    globals->capacity = new_capacity;
    free(globals->keys);
    globals->keys = new_keys;
    free(globals->values);
    globals->values = new_values;
}

static inline void Globals_delete(struct Globals* globals) {
    free(globals->keys);
    free(globals->values);
}

static inline struct Var* Globals_find(struct Globals const* globals, struct Symbol const* name) {
    size_t const hash = name->hash;

    size_t const max_index = globals->capacity - 1;
    for (size_t collisions = 0, i = hash & max_index;; ++collisions, i = (i + collisions) & max_index) {
        struct Symbol const* const key = globals->keys[i];

        if (key == name) { return globals->values[i]; }

        if (!key) { return NULL; }
    }
}

static inline void Globals_insert(struct Globals* globals, struct Symbol const* name, struct Var* var) {
    size_t const hash = name->hash;

    while (true) {
        size_t const max_index = globals->capacity - 1;
        for (size_t collisions = 0, i = hash & max_index;; ++collisions, i = (i + collisions) & max_index) {
            if (!globals->keys[i]) {
                if ((globals->count + 1) * 2 > globals->capacity) { // New load factor > 0.5
                    Globals_rehash(globals);
                    break; // continue outer loop
                } else {
                    ++globals->count;
                    globals->keys[i] = name;
                    globals->values[i] = var;
                    return;
                }
            }
        }
    }
}
