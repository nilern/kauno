#include "globals.hpp"

#include <cstdlib>
#include <cstring>

#include "symbol.hpp"

static inline Handle<Var> Var_new(State* state, Handle<void> value) {
    Var* var = (Var*)state->alloc(state->Var.data());
    *var = (Var){.value = value.oref()};
    return state->push(ORef(var));
}

void Globals::rehash() {
    size_t const new_capacity = capacity*2;

    size_t const keys_size = sizeof(Symbol const*)*new_capacity;
    Symbol const** const new_keys = (Symbol const**)malloc(keys_size);
    memset(new_keys, 0, keys_size);

    size_t const values_size = sizeof(Var*)*new_capacity;
    Var** const new_values = (Var**)malloc(values_size);
    memset(new_values, 0, values_size);

    for (size_t i = 0; i < capacity; ++i) {
        Symbol const* const key = keys[i];

        if (key) {
            size_t const hash = key->hash;

            size_t const max_index = new_capacity - 1;
            for (size_t collisions = 0, j = hash & max_index;; ++collisions, j = (j + collisions) & max_index) {
                if (!new_keys[j]) {
                    new_keys[j] = key;
                    new_values[j] = values[i];
                    break;
                }
            }
        }
    }

    capacity = new_capacity;
    free(keys);
    keys = new_keys;
    free(values);
    values = new_values;
}

Var* Globals::find(ORef<Symbol> name) const {
    size_t const hash = name.data()->hash;

    size_t const max_index = capacity - 1;
    for (size_t collisions = 0, i = hash & max_index;; ++collisions, i = (i + collisions) & max_index) {
        Symbol const* const key = keys[i];

        if (key == name.data()) { return values[i]; }

        if (!key) { return NULL; }
    }
}

void Globals::insert(ORef<Symbol> name, ORef<Var> var) {
    size_t const hash = name.data()->hash;

    while (true) {
        size_t const max_index = capacity - 1;
        for (size_t collisions = 0, i = hash & max_index;; ++collisions, i = (i + collisions) & max_index) {
            if (!keys[i]) {
                if ((count + 1) * 2 > capacity) { // New load factor > 0.5
                    rehash();
                    break; // continue outer loop
                } else {
                    ++count;
                    keys[i] = name.data();
                    values[i] = var.data();
                    return;
                }
            }
        }
    }
}
