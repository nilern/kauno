#include "symbol.h"

#include <stdlib.h>
#include <string.h>

#if __WORDSIZE == 64
size_t const FNV_PRIME = 1099511628211u;
size_t const FNV_OFFSET_BASIS = 14695981039346656037u;
#endif

// TODO: SipHash
static inline size_t Symbol_hash(char const* name, size_t name_size) {
    size_t hash = FNV_OFFSET_BASIS;
    for (size_t i = 0; i < name_size; ++i) {
        hash ^= (uint8_t)name[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

static inline struct SymbolTable SymbolTable_new() {
    size_t const capacity = 2;

    size_t const names_size = sizeof(struct Str)*capacity;
    struct Str* const names = (struct Str*)malloc(names_size);
    memset(names, 0, names_size);

    size_t const symbols_size = sizeof(ORef)*capacity;
    ORef* const symbols = (ORef*)malloc(symbols_size);
    memset(symbols, 0, symbols_size);

    return (struct SymbolTable){
        .count = 0,
        .capacity = capacity,
        .names = names,
        .symbols = symbols
    };
}

static inline void SymbolTable_rehash(struct SymbolTable* symbols) {
    size_t const new_capacity = symbols->capacity*2;

    size_t const names_size = sizeof(struct Str)*new_capacity;
    struct Str* const new_names = (struct Str*)malloc(names_size);
    memset(new_names, 0, names_size);

    size_t const symbols_size = sizeof(ORef)*new_capacity;
    ORef* const new_symbols = (ORef*)malloc(symbols_size);
    memset(new_symbols, 0, symbols_size);

    for (size_t i = 0; i < symbols->capacity; ++i) {
        struct Str const k = symbols->names[i];

        if (k.chars) {
            ORef const v = symbols->symbols[i];
            size_t const hash = ((struct Symbol*)obj_data(v))->hash;

            size_t const max_index = new_capacity - 1;
            for (size_t collisions = 0, j = hash & max_index;; ++collisions, j = (j + collisions) & max_index) {
                if (!new_names[j].chars) {
                    new_names[j] = k;
                    new_symbols[j] = v;
                    break;
                }
            }
        }
    }

    free(symbols->names);
    symbols->names = new_names;
    free(symbols->symbols);
    symbols->symbols = new_symbols;
}

static inline void SymbolTable_delete(struct SymbolTable* symbols) {
    for (size_t i = 0; i < symbols->capacity; ++i) {
        struct Str* k = &symbols->names[i];
        if (k->chars) {
            Str_delete(k);
        }
    }

    free(symbols->names);
    free(symbols->symbols);
}

static inline bool SymbolTable_get(struct State* state, size_t hash, char const* name, size_t name_size, Handle* symbol) {
    struct SymbolTable const* symbols = &state->symbols;

    size_t const max_index = symbols->capacity - 1;
    for (size_t collisions = 0, i = hash & max_index;; ++collisions, i = (i + collisions) & max_index) {
        struct Str const k = symbols->names[i];

        if (!k.chars) { break; }

        if (k.size == name_size) {
            if (strncmp(k.chars, name, name_size) == 0) {
                *symbol = State_push(state, symbols->symbols[i]);
                return true;
            }
        }
    }

    return false;
}

static inline void SymbolTable_insert(struct State* state, size_t hash, char const* name, size_t name_size, Handle symbol) {
    struct SymbolTable* symbols = &state->symbols;

    if ((symbols->count + 1) * 2 > symbols->capacity) {
        SymbolTable_rehash(symbols);
    }

    size_t const max_index = symbols->capacity - 1;
    for (size_t collisions = 0, i = hash & max_index;; ++collisions, i = (i + collisions) & max_index) {
        struct Str k = symbols->names[i];

        if (!k.chars) {
            ++symbols->count;
            symbols->names[i] = Str_dup(name, name_size);
            symbols->symbols[i] = Handle_oref(symbol);
            break;
        }
    }
}

static inline Handle Symbol_new(struct State* state, char const* name, size_t name_size) {
    size_t const hash = Symbol_hash(name, name_size);
    Handle res;

    if (!SymbolTable_get(state, hash, name, name_size, &res)) {
        struct Symbol* symbol = (struct Symbol*)alloc(&state->heap, state->Symbol);
        *symbol = (struct Symbol){
            .hash = hash,
            .name_size = name_size
        };
        memcpy(&symbol->name, name, name_size);
        res = State_push(state, oref_from_ptr(symbol));

        SymbolTable_insert(state, hash, name, name_size, res);
    }

    return res;
}
