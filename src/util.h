#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

struct Str {
    size_t size;
    char* chars;
};

static inline struct Str Str_dup(char const* chars, size_t size);

static inline void Str_delete(struct Str* str);

#endif // UTIL_H
