#ifndef POS_H
#define POS_H

#include <stddef.h>
#include <stdio.h>

struct Span {
    size_t start;
    size_t end;
};

static inline void Span_print(FILE* dest, struct Span span);

#endif // POS_H
