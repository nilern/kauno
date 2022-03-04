#ifndef POS_H
#define POS_H

#include <cstddef>
#include <cstdio>

namespace kauno {

struct Span {
    size_t start;
    size_t end;

    void print(FILE* dest) const;
};

}

#endif // POS_H
