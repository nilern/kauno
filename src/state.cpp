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
        .fields_count = 0,
        .fields = {}
    };

    Type* tmp_Bool = (Type*)malloc(sizeof(Type));
    *tmp_Bool = (Type){
        .align = alignof(bool),
        .min_size = sizeof(bool),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0,
        .fields = {}
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
        .fields_count = Type_fields_count,
        .fields = {}
    };
    tmp_Type->fields[0] = (Field){
        .type = ORef(tmp_USize),
        .offset = offsetof(Type, align),
        .size = tmp_USize->min_size,
        .inlined = tmp_USize->inlineable
    };
    tmp_Type->fields[1] = (Field){
        .type = ORef(tmp_USize),
        .offset = offsetof(Type, min_size),
        .size = tmp_USize->min_size,
        .inlined = tmp_USize->inlineable
    };
    tmp_Type->fields[2] = (Field){
        .type = ORef(tmp_Bool),
        .offset = offsetof(Type, inlineable),
        .size = tmp_Bool->min_size,
        .inlined = tmp_Bool->inlineable
    };
    tmp_Type->fields[3] = (Field){
        .type = ORef(tmp_Bool),
        .offset = offsetof(Type, is_bits),
        .size = tmp_Bool->min_size,
        .inlined = tmp_Bool->inlineable
    };
    tmp_Type->fields[4] = (Field){
        .type = ORef(tmp_Bool),
        .offset = offsetof(Type, has_indexed),
        .size = tmp_Bool->min_size,
        .inlined = tmp_Bool->inlineable
    };

    size_t const Field_fields_count = 2;
    size_t const Field_size = sizeof(Type) + Field_fields_count*sizeof(Field);
    Type* const tmp_Field = (Type*)malloc(Field_size);
    *tmp_Field = (Type){
        .align = alignof(Field),
        .min_size = sizeof(Field),
        .inlineable = true,
        .is_bits = false,
        .has_indexed = false,
        .fields_count = Field_fields_count,
        .fields = {}
    };
    tmp_Field->fields[0] = (Field){
        .type = ORef(tmp_USize),
        .offset = offsetof(Field, offset),
        .size = tmp_USize->min_size,
        .inlined = tmp_USize->inlineable
    };
    tmp_Field->fields[1] = (Field){
        .type = ORef(tmp_Type),
        .offset = offsetof(Field, type),
        .size = tmp_Type->min_size,
        .inlined = tmp_Type->inlineable
    };

    tmp_Type->fields[5] = (Field){
        .type = ORef(tmp_Field),
        .offset = offsetof(Type, fields),
        .size = tmp_Field->min_size,
        .inlined = tmp_Field->inlineable
    };


    struct Type* Type = (struct Type*)heap.alloc_indexed(tmp_Type, Type_fields_count);
    ORef(Type).set_type(ORef(Type));
    memcpy(Type, tmp_Type, Type_size);

    struct Type* Field = (struct Type*)heap.alloc_indexed(tmp_Type, Field_fields_count);
    ORef(Field).set_type(ORef(Type));
    memcpy(Field, tmp_Field, Field_size);


    struct Type* Int64 = (struct Type*)heap.alloc_indexed(Type, 0);
    *Int64 = (struct Type){
        .align = alignof(int64_t),
        .min_size = sizeof(int64_t),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0,
        .fields = {}
    };

    struct Type* USize = (struct Type*)heap.alloc_indexed(Type, 0);
    *USize = (struct Type){
        .align = alignof(size_t),
        .min_size = sizeof(size_t),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0,
        .fields = {}
    };

    struct Type* Bool = (struct Type*)heap.alloc_indexed(Type, 0);
    *Bool = (struct Type){
        .align = alignof(bool),
        .min_size = sizeof(bool),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0,
        .fields = {}
    };


    Type->fields[0].type = ORef(USize);
    Type->fields[1].type = ORef(USize);
    Type->fields[2].type = ORef(Bool);
    Type->fields[3].type = ORef(Bool);
    Type->fields[4].type = ORef(Bool);
    Type->fields[5].type = ORef(Field);

    Field->fields[0].type = ORef(USize);
    Field->fields[1].type = ORef(Type);


    free(tmp_USize);
    free(tmp_Bool);
    free(tmp_Type);
    free(tmp_Field);


    struct Type* UInt8 = (struct Type*)heap.alloc_indexed(Type, 0);
    *UInt8 = (struct Type){
        .align = alignof(uint8_t),
        .min_size = sizeof(uint8_t),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0,
        .fields = {}
    };

    size_t const Symbol_fields_count = 2;
    struct Type* Symbol = (struct Type*)heap.alloc_indexed(Type, Symbol_fields_count);
    *Symbol = (struct Type){
        .align = alignof(struct Symbol),
        .min_size = sizeof(struct Symbol),
        .inlineable = false,
        .is_bits = false,
        .has_indexed = true,
        .fields_count = Symbol_fields_count,
        .fields = {}
    };
    Symbol->fields[0] = (struct Field){
        .type = ORef(USize),
        .offset = offsetof(struct Symbol, hash),
        .size = USize->min_size,
        .inlined = USize->inlineable
    };
    Symbol->fields[1] = (struct Field){
        .type = ORef(UInt8),
        .offset = offsetof(struct Symbol, name),
        .size = UInt8->min_size,
        .inlined = UInt8->inlineable
    };

    size_t const Any_fields_count = 0;
    struct Type* Any = (struct Type*)heap.alloc_indexed(Type, Any_fields_count);
    *Any = (struct Type){
        .align = 1,
        .min_size = 0,
        .inlineable = false,
        .is_bits = false,
        .has_indexed = false,
        .fields_count = Any_fields_count,
        .fields = {}
    };

    size_t const Var_fields_count = 1;
    struct Type* Var = (struct Type*)heap.alloc_indexed(Type, Var_fields_count);
    *Var = (struct Type){
        .align = alignof(struct Var),
        .min_size = sizeof(struct Var),
        .inlineable = false,
        .is_bits = false,
        .has_indexed = false,
        .fields_count = Var_fields_count,
        .fields = {}
    };
    Var->fields[0] = (struct Field){
        .type = ORef(Any),
        .offset = offsetof(struct Var, value),
        .size = sizeof(ORef<struct Any>),
        .inlined = Any->inlineable
    };

    ORef<struct Any>* const stack = (ORef<struct Any>*)malloc(stack_size);


    State state = (State) {
        .heap = std::move(heap),

        .Type = ORef(Type),
        .UInt8 = ORef(UInt8),
        .Int64 = ORef(Int64),
        .USize = ORef(USize),
        .Bool = ORef(Bool),
        .Symbol = ORef(Symbol),
        .Any = ORef(Any),
        .Var = ORef(Var),

        .symbols = SymbolTable_new(),

        .globals = Globals_new(),

        .sp = stack,
        .stack_size = stack_size,
        .stack = stack
    };


    Handle<struct Type> const Type_handle = State_push(&state, ORef(Type));
    Handle<struct Symbol> const Type_symbol = Symbol_new(&state, "Type", 4);
    Handle<struct Var> const Type_var = Var_new(&state, Type_handle.as_any());
    Globals_insert(&state.globals, Type_symbol.oref(), Type_var.oref());
    State_popn(&state, 3);


    return state;
}

static inline void State_delete(State* state) {
    SymbolTable_delete(&state->symbols);
    Globals_delete(&state->globals);
    free(state->stack);
}

static inline Handle<Any> State_peek(State* state) {
    assert(state->sp > &state->stack[0]);
    return Handle(state->sp - 1);
}

static inline void State_pop(State* state) {
    assert(state->sp > &state->stack[0]);
    --state->sp;
}

static inline void State_popn(State* state, size_t n) {
    assert(state->sp - n >= &state->stack[0]);
    state->sp -= n;
}

static inline void State_print_builtin(State const* state, FILE* dest, Handle<Any> value) {
    ORef<Type> type = value.type();
    void* data = value.data();
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
