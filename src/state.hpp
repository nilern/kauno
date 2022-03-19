#ifndef STATE_H
#define STATE_H

#include <cstdio>
#include <utility>
#include <random>

#include "gc.hpp"
#include "symbol.hpp"
#include "globals.hpp"
#include "ast.hpp"

namespace kauno {

static inline void State_print_builtin(State const& state, FILE* dest, AnySRef value);

class State {
    gc::Heap heap;

    // TODO: Growable ("infinite") stack:
    size_t stack_size;
    char* stack;
    char* data_sp;
    DynRef* type_sp;

    std::mt19937_64 type_hashes_; // FIXME: assumes 64-bit (or less) hash size

    SymbolTable symbols_;

    Globals globals;

public:
    ORef<struct Type> Type;
    ORef<struct Type> Field;
    ORef<struct Type> UInt8;
    ORef<struct Type> Int64;
    ORef<struct Type> USize;
    ORef<struct Type> Bool;
    ORef<struct Type> Symbol;
    ORef<struct Type> Var;
    ORef<struct Type> AstFn;
    ORef<struct Type> Call;
    ORef<struct Type> CodePtr;
    ORef<struct Type> Fn;
    ORef<struct Type> Closure;
    ORef<struct Type> NoneType;
    ORef<struct Type> RefArray;
    ORef<struct Type> NRefArray;
    ORef<struct Type> TypesMap;
    ORef<struct Type> Locals;

    ORef<struct None> None;

    State(size_t heap_size, size_t stack_size);

    State(State&& semi) = delete;
    State& operator=(State&& semi) = delete;

    State(State const& semi) = delete;
    State& operator=(State const& semi) = delete;

    ~State() {
        free(stack);
    }

    // HACK:
    void* alloc(struct Type* type) { return heap.alloc(type); }
    void* alloc_indexed(struct Type* type, size_t indexed_count) { return heap.alloc_indexed(type, indexed_count); }

    size_t create_type_hash() { return type_hashes_(); }

    template<typename T>
    SRef<T> stack_alloc(ORef<struct Type> type) {
        size_t const size = type.data()->min_size;
        size_t new_data_addr = (size_t)(void*)data_sp;
        new_data_addr -= size; // TODO: overflow check
        new_data_addr &= ~(type.data()->align - 1); // Align
        void* const data = (void*)new_data_addr;

        DynRef* const old_tp = type_sp;
        DynRef* const new_tp = old_tp + 1;

        if ((void*)new_tp > data) { exit(EXIT_FAILURE); } // FIXME

        memset(data, 0, size);
        *old_tp = DynRef(type, data);

        data_sp = (char*)data;
        type_sp = new_tp;

        return SRef<T>((T*)data);
    }

    template<typename T>
    SRef<T> push_inlined(ORef<T> value) {
        ORef<struct Type> const type = value.type();
        SRef<T> const sref = stack_alloc<T>(type);
        memcpy(sref.data(), value.data(), type.data()->min_size);
        return sref;
    }

    template<typename T>
    Handle<T> push_outlined(ORef<T> value) {
        size_t new_data_addr = (size_t)(void*)data_sp;
        new_data_addr -= sizeof(ORef<T>); // TODO: overflow check
        new_data_addr &= ~(alignof(ORef<T>) - 1); // Align
        ORef<T>* const oref_ptr = (ORef<T>*)new_data_addr;

        DynRef* const old_tp = type_sp;
        DynRef* const new_tp = old_tp + 1;

        if ((void*)new_tp > oref_ptr) { exit(EXIT_FAILURE); } // FIXME

        *oref_ptr = value;
        *old_tp = DynRef(oref_ptr);

        data_sp = (char*)oref_ptr;
        type_sp = new_tp;

        return Handle<T>(oref_ptr);
    }

    AnySRef push(ORef<void> oref) {
        ORef<struct Type> const type = oref.type();

        if (type.data()->inlineable) {
            push_inlined(oref);
        } else {
            push_outlined(oref);
        }

        return peek();
    }

    AnySRef peek();

    AnySRef peek_nth(size_t n);

    DynRef* peekn(size_t n);

    void pop();

    void popn(size_t n);

    void pop_nth(size_t i);

    void popn_nth(size_t i, size_t n);

    SymbolTable* symbols() { return &symbols_; }

    struct Var* global(ORef<struct Symbol> name) { return globals.find(name); }

    void dump_stack(FILE* dest) {
        for (DynRef* p = (DynRef*)stack; p < type_sp; ++p) {
            fprintf(dest, "[%zd] = ", type_sp - p);
            State_print_builtin(*this, dest, AnySRef(p));
            fputs("\n", dest);
        }
    }
};

}

#endif // STATE_H
