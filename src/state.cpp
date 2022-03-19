#include "state.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdalign>
#include <utility>
#include <algorithm>

#include "arrays.hpp"
#include "typesmap.hpp"
#include "locals.hpp"
#include "fn.hpp"

namespace kauno {

static inline AnySRef builtin_prn(State& state) {
    State_print_builtin(state, stdout, state.peek());
    puts("");
    state.pop_nth(1); // Pop self
    return state.peek(); // FIXME
}

State::State(size_t heap_size, size_t stack_size_) :
    heap(heap_size),

    stack_size(stack_size_),
    stack((char*)malloc(stack_size)),
    data_sp(stack + stack_size),
    type_sp((DynRef*)stack),

    type_hashes_(std::mt19937_64(std::random_device()())),

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
    AstFn(ORef<struct Type>(nullptr)),
    Call(ORef<struct Type>(nullptr)),
    CodePtr(ORef<struct Type>(nullptr)),
    Fn(ORef<struct Type>(nullptr)),
    Closure(ORef<struct Type>(nullptr)),
    NoneType(ORef<struct Type>(nullptr)),
    RefArray(ORef<struct Type>(nullptr)),
    NRefArray(ORef<struct Type>(nullptr)),
    TypesMap(ORef<struct Type>(nullptr)),
    Locals(ORef<struct Type>(nullptr)),

    None(ORef<struct None>(nullptr))
{
    // Sentinel to simplify (and optimize) popping:
    *type_sp = DynRef((ORef<void>*)data_sp);
    ++type_sp;


    size_t const Type_fields_count = 6;
    size_t const Type_size = sizeof(struct Type) + Type_fields_count*sizeof(Field);
    struct Type* tmp_Type = (struct Type*)malloc(Type_size);
    *tmp_Type = Type::create_indexed(*this, alignof(struct Type), sizeof(struct Type), Type_fields_count);

    size_t const Field_fields_count = 4;
    size_t const Field_size = sizeof(struct Type) + Field_fields_count*sizeof(Field);
    struct Type* const tmp_Field = (struct Type*)malloc(Field_size);
    *tmp_Field = Type::create_record(*this, alignof(struct Field), sizeof(struct Field), true, Field_fields_count);
    tmp_Type->fields[5] = (struct Field){NRef(tmp_Field), offsetof(struct Type, fields)};


    Type = ORef((struct Type*)heap.alloc_indexed(tmp_Type, Type_fields_count));
    Type.set_type(Type);
    *Type.data() = *tmp_Type;
    Type.data()->fields[5] = tmp_Type->fields[5];

    Field = ORef((struct Type*)heap.alloc_indexed(tmp_Type, Field_fields_count));
    Field.set_type(Type);
    *Field.data() = *tmp_Field;


    USize = ORef((struct Type*)heap.alloc_indexed(Type.data(), 0));
    *USize.data() = Type::create_bits(*this, alignof(size_t), sizeof(size_t), true);

    Bool = ORef((struct Type*)heap.alloc_indexed(Type.data(), 0));
    *Bool.data() = Type::create_bits(*this, alignof(bool), sizeof(bool), true);


    Type.data()->fields[0] = (struct Field){NRef(USize), offsetof(struct Type, align)};
    Type.data()->fields[1] = (struct Field){NRef(USize), offsetof(struct Type, min_size)};
    Type.data()->fields[2] = (struct Field){NRef(Bool), offsetof(struct Type, inlineable)};
    Type.data()->fields[3] = (struct Field){NRef(Bool), offsetof(struct Type, is_bits)};
    Type.data()->fields[4] = (struct Field){NRef(Bool), offsetof(struct Type, has_indexed)};
    Type.data()->fields[5] = (struct Field){NRef(Field), offsetof(struct Type, fields)};

    Field.data()->fields[0] = (struct Field){NRef(Type), offsetof(struct Field, type)};
    Field.data()->fields[1] = (struct Field){NRef(USize), offsetof(struct Field, offset)};
    Field.data()->fields[2] = (struct Field){NRef(USize), offsetof(struct Field, size)};
    Field.data()->fields[3] = (struct Field){NRef(Bool), offsetof(struct Field, inlined)};


    free(tmp_Type);
    free(tmp_Field);


    UInt8 = ORef((struct Type*)heap.alloc_indexed(Type.data(), 0));
    *UInt8.data() = Type::create_bits(*this, alignof(uint8_t), sizeof(uint8_t), true);

    Int64 = ORef((struct Type*)heap.alloc_indexed(Type.data(), 0));
    *Int64.data() = Type::create_bits(*this, alignof(int64_t), sizeof(int64_t), true);

    size_t const Symbol_fields_count = 2;
    Symbol = ORef((struct Type*)heap.alloc_indexed(Type.data(), Symbol_fields_count));
    *Symbol.data() = Type::create_indexed(*this, alignof(struct Symbol), sizeof(struct Symbol), Symbol_fields_count);
    Symbol.data()->fields[0] = (struct Field){NRef(USize), offsetof(struct Symbol, hash)};
    Symbol.data()->fields[1] = (struct Field){NRef(UInt8), offsetof(struct Symbol, name)};

    size_t const Var_fields_count = 1;
    Var = ORef((struct Type*)heap.alloc_indexed(Type.data(), Var_fields_count));
    *Var.data() = Type::create_record(*this, alignof(struct Var), sizeof(struct Var), false, Var_fields_count);
    Var.data()->fields[0] = (struct Field){NRef<struct Type>(), offsetof(struct Var, value)};

    size_t const Call_fields_count = 2;
    Call = ORef((struct Type*)heap.alloc_indexed(Type.data(), Call_fields_count));
    *Call.data() = Type::create_indexed(*this, alignof(kauno::ast::Call), sizeof(kauno::ast::Call), Call_fields_count);
    Call.data()->fields[0] = (struct Field){NRef<struct Type>(), offsetof(kauno::ast::Call, callee)};
    Call.data()->fields[1] = (struct Field){NRef<struct Type>(), offsetof(kauno::ast::Call, args)};

    CodePtr = ORef((struct Type*)heap.alloc_indexed(Type.data(), 0));
    *CodePtr.data() = Type::create_bits(*this, alignof(kauno::fn::CodePtr), sizeof(kauno::fn::CodePtr), true);

    size_t const Fn_fields_count = 1;
    Fn = ORef((struct Type*)heap.alloc_indexed(Type.data(), Fn_fields_count));
    *Fn.data() = Type::create_record(*this, alignof(kauno::fn::Fn), sizeof(kauno::fn::Fn), true, Fn_fields_count);
    Fn.data()->fields[0] = (struct Field){NRef(CodePtr), offsetof(kauno::fn::Fn, code)};

    size_t const None_fields_count = 0;
    NoneType = ORef((struct Type*)heap.alloc_indexed(Type.data(), None_fields_count));
    *NoneType.data() = Type::create_record(*this, alignof(struct None), sizeof(struct None), true, None_fields_count);

    size_t const RefArray_fields_count = 1;
    RefArray = ORef((struct Type*)heap.alloc_indexed(Type.data(), RefArray_fields_count));
    *RefArray.data() = Type::create_indexed(*this, alignof(kauno::arrays::RefArray<ORef<void>>), sizeof(kauno::arrays::RefArray<ORef<void>>),
                                            RefArray_fields_count);
    RefArray.data()->fields[0] = (struct Field){NRef<struct Type>(),
            offsetof(kauno::arrays::RefArray<ORef<void>>, elements)};

    size_t const NRefArray_fields_count = 1;
    NRefArray = ORef((struct Type*)heap.alloc_indexed(Type.data(), NRefArray_fields_count));
    *NRefArray.data() = Type::create_indexed(*this, alignof(kauno::arrays::NRefArray<ORef<void>>), sizeof(kauno::arrays::NRefArray<ORef<void>>),
                                             NRefArray_fields_count);
    NRefArray.data()->fields[0] = (struct Field){NRef<struct Type>(),
            offsetof(kauno::arrays::NRefArray<ORef<void>>, elements)};

    TypesMap = kauno::TypesMap::create_reified(*this);

    AstFn = ast::Fn::create_reified(*this);

    Locals = Locals::create_reified(*this);

    Closure = fn::Closure::create_reified(*this);

    None = ORef(static_cast<struct None*>(heap.alloc(NoneType.data())));


    Handle<struct Type> const Type_handle = push_outlined(ORef(Type));
    Handle<struct Symbol> const Type_symbol = Symbol_new(*this, "Type", 4);
    Handle<struct Var> const Type_var = Var_new(*this, Type_handle.as_void());
    globals.insert(Type_symbol.oref(), Type_var.oref());
    popn(3);

    ORef<kauno::fn::Fn> const prn = ORef((kauno::fn::Fn*)heap.alloc(Fn.data()));
    *prn.data() = (kauno::fn::Fn){
        .code = builtin_prn,
        .domain_count = 1,
        .domain = {}
    };
    prn.data()->domain[0] = None.as_void();
    Handle<kauno::fn::Fn> const prn_handle = push_outlined(ORef(prn));
    Handle<struct Symbol> const prn_symbol = Symbol_new(*this, "prn", 3);
    Handle<struct Var> const prn_var = Var_new(*this, prn_handle.as_void());
    globals.insert(prn_symbol.oref(), prn_var.oref());
    popn(3);
}

AnySRef State::peek() {
    assert((char*)type_sp > stack);

    return AnySRef(type_sp - 1);
}

AnySRef State::peek_nth(size_t n) {
    assert((char*)type_sp - n >= stack);

    return AnySRef(type_sp - 1 - n);
}

DynRef* State::peekn(size_t n) {
    assert((char*)type_sp - n >= stack);

    return type_sp - n;
}

void State::popn(size_t n) {
    assert((char*)(type_sp - n) > stack);

    type_sp -= n;
    data_sp = (char*)((type_sp - 1)->stack_ptr());
}

void State::pop() { popn(1); }

void State::popn_nth(size_t i, size_t n) {
    assert(n <= i + 1);
    assert((char*)(type_sp - i) > stack);

    size_t const top_count = i + 1 - n;
    DynRef* it = type_sp - top_count;

    type_sp -= i + 1;
    data_sp = (char*)((type_sp - 1)->stack_ptr());

    for (size_t j = 0; j < top_count; ++j, ++it) {
        it->repush(*this);
    }
}

void State::pop_nth(size_t i) { popn_nth(i, 1); }

static inline void State_print_builtin(State const& state, FILE* dest, AnySRef value) {
    ORef<Type> type = value.type();
    void* data = value.data();
    if (type == state.Type) {
        fprintf(dest, "<Type @ %p>", data);
    } else if (type == state.Field) {
        fprintf(dest, "<Field @ %p>", data);
    } else if (type == state.Symbol) {
        Symbol* symbol = (Symbol*)data;

        fputc('\'', dest);
        for (size_t i = 0; i < symbol->name_size; ++i) {
            fputc(symbol->name[i], dest);
        }
    } else if (type == state.Var) {
        fputs("<Var>", dest);
    } else if (type == state.AstFn) {
        fprintf(dest, "<AstFn @ %p>", data);
    } else if (type == state.Call) {
        fprintf(dest, "<Call @ %p>", data);
    } else if (type == state.Fn) {
        fprintf(dest, "<Fn @ %p>", data);
    } else if (type == state.CodePtr) {
        fprintf(dest, "<CodePtr @ %p>", data);
    } else if (type == state.Closure) {
        fprintf(dest, "<Closure @ %p>", data);
    } else if (type == state.UInt8) {
        fprintf(dest, "%u", *(uint8_t*)data);
    } else if (type == state.Int64) {
        fprintf(dest, "%ld", *(int64_t*)data);
    } else if (type == state.USize) {
        fprintf(dest, "%zu", *(size_t*)data);
    } else if (type == state.Bool) {
        fputs(*(bool*)data ? "True" : "False", dest);
    } else {
        fprintf(dest, "<??? @ %p>", data);
    }
}

}
