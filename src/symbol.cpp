#include "symbol.hpp"

#include <cstdlib>

namespace kauno {

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

ORef<Type> Symbol::reify(State const& state) { return state.Symbol; }

void SymbolTable::rehash() {
    size_t const new_capacity = capacity*2;

    size_t const symbols_size = sizeof(Symbol*)*new_capacity;
    Symbol** const new_symbols = (Symbol**)malloc(symbols_size);
    memset(new_symbols, 0, symbols_size);

    for (size_t i = 0; i < capacity; ++i) {
        Symbol* const symbol = symbols[i];

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

    capacity = new_capacity;
    free(symbols);
    symbols = new_symbols;
}

static inline Handle<Symbol> Symbol_new(State& state, char const* name, size_t name_size) {
    SymbolTable* symbols = state.symbols();
    size_t const hash = Symbol_hash(name, name_size);

    while (true) {
        size_t const max_index = symbols->capacity - 1;
        for (size_t collisions = 0, i = hash & max_index;; ++collisions, i = (i + collisions) & max_index) {
            Symbol* const isymbol = symbols->symbols[i];

            if (!isymbol) { // Not found
                if ((symbols->count + 1) * 2 > symbols->capacity) { // New load factor > 0.5
                    symbols->rehash();
                    break; // continue outer loop
                } else {
                    // Construct:
                    Symbol* symbol = (Symbol*)state.alloc_indexed(state.Symbol.data(), name_size);
                    *symbol = (Symbol){
                        .hash = hash,
                        .name_size = name_size,
                        .name = {}
                    };
                    memcpy(&symbol->name, name, name_size);

                    // Insert:
                    ++symbols->count;
                    symbols->symbols[i] = symbol;

                    return state.push(ORef(symbol));
                }
            } else if (isymbol->hash == hash
                       && isymbol->name_size == name_size
                       && strncmp(isymbol->name, name, name_size) == 0)
            {
                return state.push(ORef(isymbol));
            }
        }
    }
}

}
