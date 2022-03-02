#ifndef STATE_H
#define STATE_H

#include <cstdio>
#include <utility>

#include "gc.hpp"
#include "symbol.hpp"
#include "globals.hpp"
#include "ast.hpp"

static inline void State_print_builtin(State const* state, FILE* dest, Handle<void> value);

class State {
    Heap heap;

    ORef<void>* sp;
    size_t stack_size;
    ORef<void>* stack; // TODO: Growable ("infinite") stack

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
    ORef<struct Type> Call;
    ORef<struct Type> CodePtr;
    ORef<struct Type> Fn;
    ORef<struct Type> NoneType;

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

    template<typename T>
    Handle<T> push(ORef<T> value) {
        ORef<void>* const old_sp = sp;
        ORef<void>* const new_sp = old_sp + 1;
        if (new_sp >= (ORef<void>*)((char*)&stack + stack_size)) { exit(EXIT_FAILURE); } // FIXME
        *old_sp = value.as_void();
        sp = new_sp;
        return Handle(old_sp).template unchecked_cast<T>();
    }

    Handle<void> peek();

    Handle<void> peek_nth(size_t n);

    ORef<void>* peekn(size_t n);

    void pop();

    void popn(size_t n);

    void pop_nth(size_t n);

    SymbolTable* symbols() { return &symbols_; }

    struct Var* global(Handle<struct Symbol> name) { return globals.find(name.oref()); }

    void dump_stack(FILE* dest) {
        for (ORef<void>* p = &stack[0]; p < sp; ++p) {
            fprintf(dest, "[%zd] = ", sp - p);
            State_print_builtin(this, dest, Handle(p));
            fputs("\n", dest);
        }
    }
};

template<typename T, typename F>
ORef<F> obj_field(State* state, Handle<T> handle, size_t index) {
    ORef const obj = Handle_oref(handle);

    Type* const type = obj_type(obj);

    if (index >= type->fields_count) {
        exit(EXIT_FAILURE); // FIXME
    }
    Field const field = type->fields[index];

    if (field.inlined) {
        F* const field_obj = state->alloc(field.type.data());
        obj = Handle_oref(handle); // Reload after potential collection
        memcpy(field_obj, (void*)((char*)obj_data(obj) + field.offset), field.size);
        return ORef(field_obj);
    } else {
        return *(ORef<F>*)((char*)obj_data(obj) + field.offset);
    }
}
// TODO: field_indexed

template<typename T, typename F>
static inline void obj_field_set(State*, Handle<T> handle, size_t index, Handle<F> new_val_handle) {
    ORef const obj = Handle_oref(handle);

    Type* const type = obj_type(obj);

    if (index >= type->fields_count) {
        exit(EXIT_FAILURE); // FIXME
    }
    Field const field = type->fields[index];

    ORef const new_val = Handle_oref(new_val_handle);

    // TODO: Polymorphic fields:
    if (!obj_eq(new_val.type(), field.type)) {
        exit(EXIT_FAILURE); // FIXME
    }

    if (field.inlined) {
        memcpy((void*)((char*)obj.data() + field.offset), obj_data(new_val), field.size);
    } else {
        *(ORef<F>*)((char*)obj.data() + field.offset) = new_val;
    }
}
// TODO: obj_field_indexed_set

#endif // STATE_H
