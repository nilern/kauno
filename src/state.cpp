#include "state.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdalign>
#include <utility>
#include <algorithm>

#include "fn.hpp"

static inline Handle<Any> builtin_prn(State* state) {
    State_print_builtin(state, stdout, state->peek());
    puts("");
    state->pop_nth(1); // Pop self
    return state->peek(); // FIXME
}

State::State(size_t heap_size, size_t stack_size_) :
    heap(heap_size),

    stack_size(stack_size_),
    stack((ORef<struct Any>*)malloc(stack_size_)),

    symbols_(),

    globals(),

    Type(ORef<struct Type>(nullptr)),
    Field(ORef<struct Type>(nullptr)),
    UInt8(ORef<struct Type>(nullptr)),
    Int64(ORef<struct Type>(nullptr)),
    USize(ORef<struct Type>(nullptr)),
    Bool(ORef<struct Type>(nullptr)),
    Symbol(ORef<struct Type>(nullptr)),
    Var(ORef<struct Type>(nullptr)),
    Call(ORef<struct Type>(nullptr)),
    CodePtr(ORef<struct Type>(nullptr)),
    Fn(ORef<struct Type>(nullptr)),
    NoneType(ORef<struct Type>(nullptr)),

    None(ORef<struct None>(nullptr))
{
    sp = stack;


    size_t const Type_fields_count = 6;
    size_t const Type_size = sizeof(struct Type) + Type_fields_count*sizeof(Field);
    struct Type* tmp_Type = (struct Type*)malloc(Type_size);
    *tmp_Type = (struct Type){
        .align = alignof(struct Type),
        .min_size = sizeof(struct Type),
        .inlineable = false,
        .is_bits = false,
        .has_indexed = true,
        .fields_count = Type_fields_count,
        .fields = {}
    };

    size_t const Field_fields_count = 4;
    size_t const Field_size = sizeof(struct Type) + Field_fields_count*sizeof(Field);
    struct Type* const tmp_Field = (struct Type*)malloc(Field_size);
    *tmp_Field = (struct Type){
        .align = alignof(struct Field),
        .min_size = sizeof(struct Field),
        .inlineable = true,
        .is_bits = false,
        .has_indexed = false,
        .fields_count = Field_fields_count,
        .fields = {}
    };
    tmp_Type->fields[5] = (struct Field){ORef(tmp_Field), offsetof(struct Type, fields)};


    Type = ORef((struct Type*)heap.alloc_indexed(tmp_Type, Type_fields_count));
    Type.set_type(Type);
    *Type.data() = *tmp_Type;
    Type.data()->fields[5] = tmp_Type->fields[5];

    Field = ORef((struct Type*)heap.alloc_indexed(tmp_Type, Field_fields_count));
    Field.set_type(Type);
    *Field.data() = *tmp_Field;


    USize = ORef((struct Type*)heap.alloc_indexed(Type.data(), 0));
    *USize.data() = (struct Type){
        .align = alignof(size_t),
        .min_size = sizeof(size_t),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0,
        .fields = {}
    };

    Bool = ORef((struct Type*)heap.alloc_indexed(Type.data(), 0));
    *Bool.data() = (struct Type){
        .align = alignof(bool),
        .min_size = sizeof(bool),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0,
        .fields = {}
    };


    Type.data()->fields[0] = (struct Field){USize, offsetof(struct Type, align)};
    Type.data()->fields[1] = (struct Field){USize, offsetof(struct Type, min_size)};
    Type.data()->fields[2] = (struct Field){Bool, offsetof(struct Type, inlineable)};
    Type.data()->fields[3] = (struct Field){Bool, offsetof(struct Type, is_bits)};
    Type.data()->fields[4] = (struct Field){Bool, offsetof(struct Type, has_indexed)};
    Type.data()->fields[5] = (struct Field){Field, offsetof(struct Type, fields)};

    Field.data()->fields[0] = (struct Field){Type, offsetof(struct Field, type)};
    Field.data()->fields[1] = (struct Field){USize, offsetof(struct Field, offset)};
    Field.data()->fields[2] = (struct Field){USize, offsetof(struct Field, size)};
    Field.data()->fields[3] = (struct Field){Bool, offsetof(struct Field, inlined)};


    free(tmp_Type);
    free(tmp_Field);


    UInt8 = ORef((struct Type*)heap.alloc_indexed(Type.data(), 0));
    *UInt8.data() = (struct Type){
        .align = alignof(uint8_t),
        .min_size = sizeof(uint8_t),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0,
        .fields = {}
    };

    Int64 = ORef((struct Type*)heap.alloc_indexed(Type.data(), 0));
    *Int64.data() = (struct Type){
        .align = alignof(int64_t),
        .min_size = sizeof(int64_t),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0,
        .fields = {}
    };

    size_t const Symbol_fields_count = 2;
    Symbol = ORef((struct Type*)heap.alloc_indexed(Type.data(), Symbol_fields_count));
    *Symbol.data() = (struct Type){
        .align = alignof(struct Symbol),
        .min_size = sizeof(struct Symbol),
        .inlineable = false,
        .is_bits = false,
        .has_indexed = true,
        .fields_count = Symbol_fields_count,
        .fields = {}
    };
    Symbol.data()->fields[0] = (struct Field){ORef(USize), offsetof(struct Symbol, hash)};
    Symbol.data()->fields[1] = (struct Field){ORef(UInt8), offsetof(struct Symbol, name)};

    size_t const Var_fields_count = 1;
    Var = ORef((struct Type*)heap.alloc_indexed(Type.data(), Var_fields_count));
    *Var.data() = (struct Type){
        .align = alignof(struct Var),
        .min_size = sizeof(struct Var),
        .inlineable = false,
        .is_bits = false,
        .has_indexed = false,
        .fields_count = Var_fields_count,
        .fields = {}
    };
    Var.data()->fields[0] = (struct Field){ORef<struct Type>(nullptr), offsetof(struct Var, value)};

    size_t const Call_fields_count = 2;
    Call = ORef((struct Type*)heap.alloc_indexed(Type.data(), Call_fields_count));
    *Call.data() = (struct Type){
        .align = alignof(struct Call),
        .min_size = sizeof(struct Call),
        .inlineable = false,
        .is_bits = false,
        .has_indexed = true,
        .fields_count = Call_fields_count,
        .fields = {}
    };
    Call.data()->fields[0] = (struct Field){ORef<struct Type>(nullptr), offsetof(struct Call, callee)};
    Call.data()->fields[1] = (struct Field){ORef<struct Type>(nullptr), offsetof(struct Call, args)};

    CodePtr = ORef((struct Type*)heap.alloc_indexed(Type.data(), 0));
    *CodePtr.data() = (struct Type){
        .align = alignof(kauno::fn::CodePtr),
        .min_size = sizeof(kauno::fn::CodePtr),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0,
        .fields = {}
    };

    size_t const Fn_fields_count = 1;
    Fn = ORef((struct Type*)heap.alloc_indexed(Type.data(), Fn_fields_count));
    *Fn.data() = (struct Type){
        .align = alignof(kauno::fn::Fn),
        .min_size = sizeof(kauno::fn::Fn),
        .inlineable = true,
        .is_bits = false,
        .has_indexed = false,
        .fields_count = Fn_fields_count,
        .fields = {}
    };
    Fn.data()->fields[0] = (struct Field){CodePtr, offsetof(kauno::fn::Fn, code)};

    size_t const None_fields_count = 0;
    NoneType = ORef((struct Type*)heap.alloc_indexed(Type.data(), None_fields_count));
    *NoneType.data() = (struct Type){
        .align = alignof(struct None),
        .min_size = sizeof(struct None),
        .inlineable = true,
        .is_bits = false,
        .has_indexed = false,
        .fields_count = None_fields_count,
        .fields = {}
    };

    None = ORef(static_cast<struct None*>(heap.alloc(NoneType.data())));


    Handle<struct Type> const Type_handle = push(ORef(Type));
    Handle<struct Symbol> const Type_symbol = Symbol_new(this, "Type", 4);
    Handle<struct Var> const Type_var = Var_new(this, Type_handle.as_any());
    globals.insert(Type_symbol.oref(), Type_var.oref());
    popn(3);

    ORef<kauno::fn::Fn> const prn = ORef((kauno::fn::Fn*)heap.alloc(Fn.data()));
    *prn.data() = (kauno::fn::Fn){
        .code = builtin_prn,
        .domain_count = 1,
        .domain = {}
    };
    prn.data()->domain[0] = None.as_any();
    Handle<kauno::fn::Fn> const prn_handle = push(ORef(prn));
    Handle<struct Symbol> const prn_symbol = Symbol_new(this, "prn", 3);
    Handle<struct Var> const prn_var = Var_new(this, prn_handle.as_any());
    globals.insert(prn_symbol.oref(), prn_var.oref());
    popn(3);
}

Handle<Any> State::peek() {
    assert(sp > &stack[0]);
    return Handle(sp - 1);
}

Handle<Any> State::peek_nth(size_t n) {
    assert(sp - n >= &stack[0]);
    return Handle(sp - 1 - n);
}

ORef<Any>* State::peekn(size_t n) {
    assert(sp - n >= &stack[0]);
    return sp - n;
}

void State::pop() {
    assert(sp > &stack[0]);
    --sp;
}

void State::popn(size_t n) {
    assert(sp - n >= &stack[0]);
    sp -= n;
}

void State::pop_nth(size_t n) {
    assert(sp - n >= &stack[0]);
    memmove(sp - 1 - n, sp - n, sizeof(ORef<struct Any>)*n);
    --sp;
}

static inline void State_print_builtin(State const* state, FILE* dest, Handle<Any> value) {
    ORef<Type> type = value.type();
    void* data = value.data();
    if (type == state->Type) {
        fprintf(dest, "<Type @ %p>", data);
    } else if (type == state->Field) {
        fprintf(dest, "<Field @ %p>", data);
    } else if (type == state->Symbol) {
        Symbol* symbol = (Symbol*)data;

        fputc('\'', dest);
        for (size_t i = 0; i < symbol->name_size; ++i) {
            fputc(symbol->name[i], dest);
        }
    } else if (type == state->Var) {
        fputs("<Var>", dest);
    } else if (type == state->Call) {
        fprintf(dest, "<Call @ %p>", data);
    } else if (type == state->Fn) {
        fprintf(dest, "<Fn @ %p>", data);
    } else if (type == state->CodePtr) {
        fprintf(dest, "<CodePtr @ %p>", data);
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
