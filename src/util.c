#include "util.h"

#include <stdlib.h>
#include <string.h>

static inline struct Str Str_dup(char const* chars, size_t size) {
    char* const new_chars = (char*)malloc(size + 1);
    strncpy(new_chars, chars, size);
    new_chars[size] = '\0';
    return (struct Str){
        .size = size,
        .chars = new_chars
    };
}

static inline void Str_delete(struct Str* str) {
    free(str->chars);
}
