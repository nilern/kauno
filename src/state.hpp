#ifndef STATE_H
#define STATE_H

#include <cstdio>

#include "gc.hpp"
#include "symbol.hpp"
#include "globals.hpp"
#include "ast.hpp"

struct State {
    Heap heap;

    ORef<struct Type> Type;
    ORef<struct Type> UInt8;
    ORef<struct Type> Int64;
    ORef<struct Type> USize;
    ORef<struct Type> Bool;
    ORef<struct Type> Symbol;
    ORef<struct Type> Any;
    ORef<struct Type> Var;
    ORef<struct Type> Call;
    ORef<struct Type> CodePtr;
    ORef<struct Type> Fn;

    SymbolTable symbols;

    Globals globals;

    ORef<struct Any>* sp;
    size_t stack_size;
    ORef<struct Any>* stack; // TODO: Growable ("infinite") stack
};

static inline State State_new(size_t heap_size, size_t stack_size);

static inline void State_delete(State* state);

template<typename T>
static inline Handle<T> State_push(State* state, ORef<T> value) {
    ORef<Any>* const sp = state->sp;
    ORef<Any>* const new_sp = sp + 1;
    if (new_sp >= (ORef<Any>*)((char*)state + state->stack_size)) { exit(EXIT_FAILURE); } // FIXME
    *sp = value.as_any();
    state->sp = new_sp;
    return Handle(sp).template unchecked_cast<T>();
}

static inline Handle<Any> State_peek(State* state);

static inline Handle<Any> State_peek_nth(State* state, size_t n);

static inline ORef<Any>* State_peekn(State* state, size_t n);

static inline void State_pop(State* state);

static inline void State_popn(State* state, size_t n);

static inline void State_pop_nth(State* state, size_t n);

static inline void State_print_builtin(State const* state, FILE* dest, Handle<Any> value);

template<typename T, typename F>
ORef<F> obj_field(State* state, Handle<T> handle, size_t index) {
    ORef const obj = Handle_oref(handle);

    Type* const type = obj_type(obj);

    if (index >= type->fields_count) {
        exit(EXIT_FAILURE); // FIXME
    }
    Field const field = type->fields[index];

    if (field.inlined) {
        F* const field_obj = state->heap.alloc(field.type.data());
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
