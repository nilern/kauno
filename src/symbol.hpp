#ifndef SYMBOL_H
#define SYMBOL_H

#include <cstddef>
#include <cstring>

#include "state.hpp"

struct Symbol {
    size_t hash;
    size_t name_size;
    char name[0];
};

static inline Handle<Symbol> Symbol_new(State* state, char const* name, size_t name_size);

class SymbolTable {
    size_t count;
    size_t capacity;
    Symbol** symbols;

public:
    SymbolTable() : count(0), capacity(2), symbols(nullptr) {
        size_t const symbols_size = sizeof(Symbol*)*capacity;
        symbols = (Symbol**)malloc(symbols_size);
        memset(symbols, 0, symbols_size);
    }

    SymbolTable(SymbolTable const&) = delete;
    SymbolTable& operator=(SymbolTable const&) = delete;

    SymbolTable(SymbolTable&&) = delete;
    SymbolTable& operator=(SymbolTable&&) = delete;

    ~SymbolTable() {
        free(symbols);
    }

private:
    void rehash();

    friend Handle<Symbol> Symbol_new(State* state, char const* name, size_t name_size);
};

#endif // SYMBOL_H
