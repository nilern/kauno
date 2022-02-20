#include "state.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdalign>
#include <utility>

static inline State State_new(size_t heap_size, size_t stack_size) {
    Heap heap(heap_size);


    Type* tmp_USize = (Type*)malloc(sizeof(Type));
    *tmp_USize = (Type){
        .align = alignof(size_t),
        .min_size = sizeof(size_t),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0
    };

    Type* tmp_Bool = (Type*)malloc(sizeof(Type));
    *tmp_Bool = (Type){
        .align = alignof(bool),
        .min_size = sizeof(bool),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0
    };

    size_t const Type_fields_count = 6;
    size_t const Type_size = sizeof(Type) + Type_fields_count*sizeof(Field);
    Type* tmp_Type = (Type*)malloc(Type_size);
    *tmp_Type = (Type){
            .align = alignof(Type),
            .min_size = sizeof(Type),
            .inlineable = false,
            .is_bits = false,
            .has_indexed = true,
            .fields_count = Type_fields_count
    };
    tmp_Type->fields[0] = (Field){
        .offset = offsetof(Type, align),
        .type = oref_from_ptr(tmp_USize)
    };
    tmp_Type->fields[1] = (Field){
        .offset = offsetof(Type, min_size),
        .type = oref_from_ptr(tmp_USize)
    };
    tmp_Type->fields[2] = (Field){
        .offset = offsetof(Type, inlineable),
        .type = oref_from_ptr(tmp_Bool)
    };
    tmp_Type->fields[3] = (Field){
        .offset = offsetof(Type, is_bits),
        .type = oref_from_ptr(tmp_Bool)
    };
    tmp_Type->fields[4] = (Field){
        .offset = offsetof(Type, has_indexed),
        .type = oref_from_ptr(tmp_Bool)
    };
    tmp_Type->fields[5] = (Field){
        .offset = offsetof(Type, fields_count),
        .type = oref_from_ptr(tmp_USize)
    };

    struct Type* Type = (struct Type*)heap.alloc_indexed(tmp_Type, Type_fields_count);
    obj_set_type(oref_from_ptr(Type), oref_from_ptr(Type));
    memcpy(Type, tmp_Type, Type_size);


    struct Type* Int64 = (struct Type*)heap.alloc_indexed(Type, 0);
    *Int64 = (struct Type){
        .align = alignof(int64_t),
        .min_size = sizeof(int64_t),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0
    };

    struct Type* USize = (struct Type*)heap.alloc_indexed(Type, 0);
    *USize = (struct Type){
        .align = alignof(size_t),
        .min_size = sizeof(size_t),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0
    };

    struct Type* Bool = (struct Type*)heap.alloc_indexed(Type, 0);
    *Bool = (struct Type){
        .align = alignof(bool),
        .min_size = sizeof(bool),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0
    };


    Type->fields[0].type = oref_from_ptr(USize);
    Type->fields[1].type = oref_from_ptr(USize);
    Type->fields[2].type = oref_from_ptr(Bool);
    Type->fields[3].type = oref_from_ptr(Bool);
    Type->fields[4].type = oref_from_ptr(Bool);
    Type->fields[5].type = oref_from_ptr(USize);


    free(tmp_USize);
    free(tmp_Bool);
    free(tmp_Type);


    struct Type* UInt8 = (struct Type*)heap.alloc_indexed(Type, 0);
    *USize = (struct Type){
        .align = alignof(uint8_t),
        .min_size = sizeof(uint8_t),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0
    };

    size_t const Symbol_fields_count = 2;
    struct Type* Symbol = (struct Type*)heap.alloc_indexed(Type, Symbol_fields_count);
    *Symbol = (struct Type){
        .align = alignof(struct Symbol),
        .min_size = sizeof(struct Symbol),
        .inlineable = false,
        .is_bits = false,
        .has_indexed = true,
        .fields_count = Symbol_fields_count
    };
    Symbol->fields[0] = (struct Field){
        .offset = offsetof(struct Symbol, hash),
        .type = oref_from_ptr(USize)
    };
    Symbol->fields[1] = (struct Field){
        .offset = offsetof(struct Symbol, name_size),
        .type = oref_from_ptr(UInt8)
    };

    size_t const Any_fields_count = 0;
    struct Type* Any = (struct Type*)heap.alloc_indexed(Type, Any_fields_count);
    *Any = (struct Type){
        .align = 1,
        .min_size = 0,
        .inlineable = false,
        .is_bits = false,
        .has_indexed = false,
        .fields_count = Any_fields_count
    };

    size_t const Var_fields_count = 1;
    struct Type* Var = (struct Type*)heap.alloc_indexed(Type, Var_fields_count);
    *Var = (struct Type){
        .align = alignof(struct Var),
        .min_size = sizeof(struct Var),
        .inlineable = false,
        .is_bits = false,
        .has_indexed = false,
        .fields_count = Var_fields_count
    };
    Var->fields[0] = (struct Field){
        .offset = offsetof(struct Var, value),
        .type = oref_from_ptr(Any)
    };

    ORef* const stack = (ORef*)malloc(stack_size);


    State state = (State) {
        .heap = std::move(heap),

        .Type = Type,
        .UInt8 = UInt8,
        .Int64 = Int64,
        .USize = USize,
        .Bool = Bool,
        .Symbol = Symbol,
        .Any = Any,
        .Var = Var,

        .symbols = SymbolTable_new(),

        .globals = Globals_new(),

        .sp = stack,
        .stack_size = stack_size,
        .stack = stack
    };


    Handle const Type_handle = State_push(&state, oref_from_ptr(Type));
    Handle const Type_symbol = Symbol_new(&state, "Type", 4);
    Handle const Type_var = Var_new(&state, Type_handle);
    Globals_insert(&state.globals, (struct Symbol*)obj_data(Handle_oref(Type_symbol)),
                   (struct Var*)obj_data(Handle_oref(Type_var)));
    State_popn(&state, 3);


    return state;
}

static inline void State_delete(State* state) {
    SymbolTable_delete(&state->symbols);
    Globals_delete(&state->globals);
    free(state->stack);
}

static inline Handle State_push(State* state, ORef value) {
    ORef* const sp = state->sp;
    ORef* const new_sp = sp + 1;
    if (new_sp >= (ORef*)((char*)state + state->stack_size)) { exit(EXIT_FAILURE); } // FIXME
    *sp = value;
    state->sp = new_sp;
    return (Handle){sp};
}

static inline Handle State_peek(State* state) {
    assert(state->sp > &state->stack[0]);
    return (Handle){state->sp - 1};
}

static inline void State_pop(State* state) {
    assert(state->sp > &state->stack[0]);
    --state->sp;
}

static inline void State_popn(State* state, size_t n) {
    assert(state->sp - n >= &state->stack[0]);
    state->sp -= n;
}

static inline void State_print_builtin(State const* state, FILE* dest, Handle value) {
    Type* type = obj_type(Handle_oref(value));
    void* data = obj_data(Handle_oref(value));
    if (type == state->Type) {
        fputs("<Type>", dest);
    } else if (type == state->Symbol) {
        Symbol* symbol = (Symbol*)data;

        fputc('\'', dest);
        for (size_t i = 0; i < symbol->name_size; ++i) {
            fputc(symbol->name[i], dest);
        }
    } else if (type == state->Var) {
        fputs("<Var>", dest);
    } else if (type == state->UInt8) {
        fprintf(dest, "%u", *(uint8_t*)data);
    } else if (type == state->Int64) {
        fprintf(dest, "%ld", *(int64_t*)data);
    } else if (type == state->USize) {
        fprintf(dest, "%zu", *(size_t*)data);
    } else if (type == state->Bool) {
        fputs(*(bool*)data ? "True" : "False", dest);
    } else {
        fprintf(dest, "<??? @ %p>", data);
    }
}
