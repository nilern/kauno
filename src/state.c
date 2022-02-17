#include "state.h"

#include <assert.h>
#include <stdlib.h>
#include <stdalign.h>

static inline struct State State_new(size_t heap_size, size_t stack_size) {
    struct Heap heap = Heap_new(heap_size);


    struct Type* tmp_USize = malloc(sizeof(struct Type));
    *tmp_USize = (struct Type){
        .align = alignof(size_t),
        .min_size = sizeof(size_t),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0
    };

    struct Type* tmp_Bool = malloc(sizeof(struct Type));
    *tmp_Bool = (struct Type){
        .align = alignof(bool),
        .min_size = sizeof(bool),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0
    };

    size_t const Type_fields_count = 6;
    size_t const Type_size = sizeof(struct Type) + Type_fields_count*sizeof(struct Field);
    struct Type* tmp_Type = malloc(Type_size);
    *tmp_Type = (struct Type){
            .align = alignof(struct Type),
            .min_size = sizeof(struct Type),
            .inlineable = false,
            .is_bits = false,
            .has_indexed = true,
            .fields_count = Type_fields_count
    };
    tmp_Type->fields[0] = (struct Field){
        .offset = offsetof(struct Type, align),
        .type = oref_from_ptr(tmp_USize)
    };
    tmp_Type->fields[1] = (struct Field){
        .offset = offsetof(struct Type, min_size),
        .type = oref_from_ptr(tmp_USize)
    };
    tmp_Type->fields[2] = (struct Field){
        .offset = offsetof(struct Type, inlineable),
        .type = oref_from_ptr(tmp_Bool)
    };
    tmp_Type->fields[3] = (struct Field){
        .offset = offsetof(struct Type, is_bits),
        .type = oref_from_ptr(tmp_Bool)
    };
    tmp_Type->fields[4] = (struct Field){
        .offset = offsetof(struct Type, has_indexed),
        .type = oref_from_ptr(tmp_Bool)
    };
    tmp_Type->fields[5] = (struct Field){
        .offset = offsetof(struct Type, fields_count),
        .type = oref_from_ptr(tmp_USize)
    };

    struct Type* Type = alloc_indexed(&heap, tmp_Type, Type_fields_count);
    obj_set_type(oref_from_ptr(Type), oref_from_ptr(Type));
    memcpy(Type, tmp_Type, Type_size);


    struct Type* Int64 = alloc_indexed(&heap, Type, 0);
    *Int64 = (struct Type){
        .align = alignof(int64_t),
        .min_size = sizeof(int64_t),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0
    };

    struct Type* USize = alloc_indexed(&heap, Type, 0);
    *USize = (struct Type){
        .align = alignof(size_t),
        .min_size = sizeof(size_t),
        .inlineable = true,
        .is_bits = true,
        .has_indexed = false,
        .fields_count = 0
    };

    struct Type* Bool = alloc_indexed(&heap, Type, 0);
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


    free(tmp_Type);
    free(tmp_Bool);
    free(tmp_USize);


    ORef* const stack = (ORef*)malloc(stack_size);


    return (struct State) {
        .heap = heap,

        .Type = Type,
        .Int64 = Int64,
        .USize = USize,
        .Bool = Bool,

        .sp = stack,
        .stack_size = stack_size,
        .stack = stack
    };
}

static inline void State_delete(struct State* state) {
    Heap_delete(&state->heap);
    free(state->stack);
}

static inline void State_push(struct State* state, ORef value) {
    ORef* const sp = state->sp;
    ORef* const new_sp = sp + 1;
    if (new_sp >= (ORef*)((char*)state + state->stack_size)) { exit(EXIT_FAILURE); } // FIXME
    *sp = value;
    state->sp = new_sp;
}

static inline ORef* State_peek(struct State* state) {
    assert(state->sp > &state->stack[0]);
    return state->sp - 1;
}

static inline void State_pop(struct State* state) {
    assert(state->sp > &state->stack[0]);
    --state->sp;
}

static inline void State_print_builtin(struct State const* state, FILE* dest, ORef* value) {
    struct Type* type = obj_type(*value);
    void* data = obj_data(*value);
    if (type == state->Type) {
        fputs("<Type>", dest);
    } else if (type == state->Int64) {
        fprintf(dest, "%ld", *(int64_t*)data);
    } else if (type == state->USize) {
        fprintf(dest, "%ld", *(size_t*)data);
    } else if (type == state->Bool) {
        fputs(*(bool*)data ? "True" : "False", dest);
    } else {
        fprintf(dest, "<??? @ %p>", data);
    }
}
