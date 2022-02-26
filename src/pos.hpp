#ifndef POS_H
#define POS_H

#include <cstddef>
#include <cstdio>

struct Span {
    size_t start;
    size_t end;

    void print(FILE* dest) const;
};

#endif // POS_H
