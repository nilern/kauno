#include "state.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdalign>
#include <utility>

#include "fn.hpp"

static inline Handle<Any> builtin_prn(State* state) {
    State_print_builtin(state, stdout, State_peek(state));
    puts("");
    State_pop_nth(state, 1);
    return State_peek(state); // FIXME
}

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
    tmp_Type->fields[0] = Field(ORef(tmp_USize), offsetof(Type, align));
    tmp_Type->fields[1] = Field(ORef(tmp_USize), offsetof(Type, min_size));
    tmp_Type->fields[2] = Field(ORef(tmp_Bool), offsetof(Type, inlineable));
    tmp_Type->fields[3] = Field(ORef(tmp_Bool), offsetof(Type, is_bits));
    tmp_Type->fields[4] = Field(ORef(tmp_Bool), offsetof(Type, has_indexed));

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
    tmp_Field->fields[0] = Field(ORef(tmp_USize), offsetof(Field, offset));
    tmp_Field->fields[1] = Field(ORef(tmp_Type), offsetof(Field, type));

    tmp_Type->fields[5] = Field(ORef(tmp_Field), offsetof(Type, fields));


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
    Symbol->fields[0] = (struct Field){ORef(USize), offsetof(struct Symbol, hash)};
    Symbol->fields[1] = (struct Field){ORef(UInt8), offsetof(struct Symbol, name)};

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
    Var->fields[0] = (struct Field){ORef(Any), offsetof(struct Var, value)};

    size_t const Call_fields_count = 2;
    struct Type* Call = (struct Type*)heap.alloc_indexed(Type, Call_fields_count);
    *Call = (struct Type){
        .align = alignof(struct Call),
        .min_size = sizeof(struct Call),
        .inlineable = false,
        .is_bits = false,
        .has_indexed = true,
        .fields_count = Call_fields_count,
        .fields = {}
    };
    Call->fields[0] = (struct Field){ORef(Any), offsetof(struct Call, callee)};
    Call->fields[1] = (struct Field){ORef(Any), offsetof(struct Call, args)};

    struct Type* CodePtr_ = (struct Type*)heap.alloc_indexed(Type, 0);
    *CodePtr_ = (struct Type){
        .align = alignof(CodePtr),
        .min_size = sizeof(CodePtr),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0,
        .fields = {}
    };

    size_t const Fn_fields_count = 1;
    struct Type* Fn = (struct Type*)heap.alloc_indexed(Type, Fn_fields_count);
    *Fn = (struct Type){
        .align = alignof(struct Fn),
        .min_size = sizeof(struct Fn),
        .inlineable = true,
        .is_bits = false,
        .has_indexed = false,
        .fields_count = Fn_fields_count,
        .fields = {}
    };
    Fn->fields[0] = (struct Field){ORef(CodePtr_), offsetof(struct Fn, code)};

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
        .Call = ORef(Call),
        .CodePtr = ORef(CodePtr_),
        .Fn = ORef(Fn),

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

    struct Fn* const prn = (struct Fn*)state.heap.alloc(Fn);
    *prn = (struct Fn){.code = builtin_prn};
    Handle<struct Fn> const prn_handle = State_push(&state, ORef(prn));
    Handle<struct Symbol> const prn_symbol = Symbol_new(&state, "prn", 3);
    Handle<struct Var> const prn_var = Var_new(&state, prn_handle.as_any());
    Globals_insert(&state.globals, prn_symbol.oref(), prn_var.oref());
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

static inline Handle<Any> State_peek_nth(State* state, size_t n) {
    assert(state->sp - n >= &state->stack[0]);
    return Handle(state->sp - 1 - n);
}

static inline ORef<Any>* State_peekn(State* state, size_t n) {
    assert(state->sp - n >= &state->stack[0]);
    return state->sp - n;
}

static inline void State_pop(State* state) {
    assert(state->sp > &state->stack[0]);
    --state->sp;
}

static inline void State_popn(State* state, size_t n) {
    assert(state->sp - n >= &state->stack[0]);
    state->sp -= n;
}

static inline void State_pop_nth(State* state, size_t n) {
    assert(state->sp - n >= &state->stack[0]);
    memmove(state->sp - 1 - n, state->sp - n, sizeof(ORef<Any>)*n);
    --state->sp;
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
