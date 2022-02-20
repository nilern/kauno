#ifndef SYMBOL_H
#define SYMBOL_H

#include <cstddef>

#include "state.hpp"

struct Symbol {
    size_t hash;
    size_t name_size;
    char name[];
};

static inline Handle<Symbol> Symbol_new(State* state, char const* name, size_t name_size);

struct SymbolTable {
    size_t count;
    size_t capacity;
    Symbol** symbols;
};

static inline SymbolTable SymbolTable_new(void);

static inline void SymbolTable_delete(SymbolTable* symbols);

#endif // SYMBOL_H
