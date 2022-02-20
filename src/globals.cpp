#include "globals.hpp"

#include <cstdlib>
#include <cstring>

#include "symbol.hpp"

static inline Handle<Var> Var_new(State* state, Handle<Any> value) {
    Var* var = (Var*)state->heap.alloc(state->Var.data());
    *var = (Var){.value = value.oref()};
    return State_push(state, ORef(var));
}

static inline Globals Globals_new() {
    size_t const capacity = 2;

    size_t const keys_size = sizeof(Symbol const*)*capacity;
    Symbol const** const keys = (Symbol const**)malloc(keys_size);
    memset(keys, 0, keys_size);

    size_t const values_size = sizeof(Var*)*capacity;
    Var** const values = (Var**)malloc(values_size);
    memset(values, 0, values_size);

    return (Globals){
        .count = 0,
        .capacity = capacity,
        .keys = keys,
        .values = values
    };
}

static inline void Globals_rehash(Globals* globals) {
    size_t const new_capacity = globals->capacity*2;

    size_t const keys_size = sizeof(Symbol const*)*new_capacity;
    Symbol const** const new_keys = (Symbol const**)malloc(keys_size);
    memset(new_keys, 0, keys_size);

    size_t const values_size = sizeof(Var*)*new_capacity;
    Var** const new_values = (Var**)malloc(values_size);
    memset(new_values, 0, values_size);

    for (size_t i = 0; i < globals->capacity; ++i) {
        Symbol const* const key = globals->keys[i];

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

static inline void Globals_delete(Globals* globals) {
    free(globals->keys);
    free(globals->values);
}

static inline Var* Globals_find(Globals const* globals, ORef<Symbol> name) {
    size_t const hash = name.data()->hash;

    size_t const max_index = globals->capacity - 1;
    for (size_t collisions = 0, i = hash & max_index;; ++collisions, i = (i + collisions) & max_index) {
        Symbol const* const key = globals->keys[i];

        if (key == name.data()) { return globals->values[i]; }

        if (!key) { return NULL; }
    }
}

static inline void Globals_insert(Globals* globals, ORef<Symbol> name, ORef<Var> var) {
    size_t const hash = name.data()->hash;

    while (true) {
        size_t const max_index = globals->capacity - 1;
        for (size_t collisions = 0, i = hash & max_index;; ++collisions, i = (i + collisions) & max_index) {
            if (!globals->keys[i]) {
                if ((globals->count + 1) * 2 > globals->capacity) { // New load factor > 0.5
                    Globals_rehash(globals);
                    break; // continue outer loop
                } else {
                    ++globals->count;
                    globals->keys[i] = name.data();
                    globals->values[i] = var.data();
                    return;
                }
            }
        }
    }
}
