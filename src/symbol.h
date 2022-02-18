#ifndef SYMBOL_H
#define SYMBOL_H

#include <stddef.h>

#include "state.h"

struct Symbol {
    size_t hash;
    size_t name_size;
    char name[];
};

static inline Handle Symbol_new(struct State* state, char const* name, size_t name_size);

struct SymbolTable {
    size_t count;
    size_t capacity;
    struct Symbol** symbols;
};

static inline struct SymbolTable SymbolTable_new(void);

static inline void SymbolTable_delete(struct SymbolTable* symbols);

#endif // SYMBOL_H
