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

    size_t const symbols_size = sizeof(struct Symbol*)*capacity;
    struct Symbol** const symbols = (struct Symbol**)malloc(symbols_size);
    memset(symbols, 0, symbols_size);

    return (struct SymbolTable){
        .count = 0,
        .capacity = capacity,
        .symbols = symbols
    };
}

static inline void SymbolTable_rehash(struct SymbolTable* symbols) {
    size_t const new_capacity = symbols->capacity*2;

    size_t const symbols_size = sizeof(struct Symbol*)*new_capacity;
    struct Symbol** const new_symbols = (struct Symbol**)malloc(symbols_size);
    memset(new_symbols, 0, symbols_size);

    for (size_t i = 0; i < symbols->capacity; ++i) {
        struct Symbol* const symbol = symbols->symbols[i];

        if (symbol) {
            size_t const hash = symbol->hash;

            size_t const max_index = new_capacity - 1;
            for (size_t collisions = 0, j = hash & max_index;; ++collisions, j = (j + collisions) & max_index) {
                if (!new_symbols[j]) {
                    new_symbols[j] = symbol;
                    break;
                }
            }
        }
    }

    free(symbols->symbols);
    symbols->symbols = new_symbols;
}

static inline void SymbolTable_delete(struct SymbolTable* symbols) {
    free(symbols->symbols);
}

static inline Handle Symbol_new(struct State* state, char const* name, size_t name_size) {
    struct SymbolTable* symbols = &state->symbols;
    size_t const hash = Symbol_hash(name, name_size);

    size_t const max_index = symbols->capacity - 1;
    for (size_t collisions = 0, i = hash & max_index;; ++collisions, i = (i + collisions) & max_index) {
        struct Symbol* const isymbol = symbols->symbols[i];

        if (!isymbol) { // Not found
            if ((symbols->count + 1) * 2 > symbols->capacity) { // New load factor > 0.5
                SymbolTable_rehash(symbols);
                return Symbol_new(state, name, name_size); // `i` will be off after rehash
            } else {
                // Construct:
                struct Symbol* symbol = (struct Symbol*)alloc(&state->heap, state->Symbol);
                *symbol = (struct Symbol){
                    .hash = hash,
                    .name_size = name_size
                };
                memcpy(&symbol->name, name, name_size);

                // Insert:
                ++symbols->count;
                symbols->symbols[i] = symbol;

                return State_push(state, oref_from_ptr(symbol));
            }
        } else if (isymbol->hash == hash
                   && isymbol->name_size == name_size
                   && strncmp(isymbol->name, name, name_size))
        {
            return State_push(state, oref_from_ptr(isymbol));
        }
    }
}