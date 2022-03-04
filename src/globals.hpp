#ifndef GLOBALS_H
#define GLOBALS_H

#include <cstddef>

#include "object.hpp"
#include "symbol.hpp"

namespace kauno {

struct Var {
    ORef<void> value;
};

static inline Handle<Var> Var_new(State* state, Handle<void> value);

class Globals {
    size_t count;
    size_t capacity;
    Symbol const** keys;
    Var** values;

public:
    Globals() : count(0), capacity(2) {
        size_t const keys_size = sizeof(Symbol const*)*capacity;
        keys = (Symbol const**)malloc(keys_size);
        memset(keys, 0, keys_size);

        size_t const values_size = sizeof(Var*)*capacity;
        values = (Var**)malloc(values_size);
        memset(values, 0, values_size);
    }

    Globals(Globals const&) = delete;
    Globals& operator=(Globals const&) = delete;

    Globals(Globals&&) = delete;
    Globals& operator=(Globals&&) = delete;

    ~Globals() {
        free(keys);
        free(values);
    }

    Var* find(ORef<Symbol> name) const;

    void insert(ORef<Symbol> name, ORef<Var> var);

private:
    void rehash();
};

}

#endif // GLOBALS_H
