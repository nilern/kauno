#include "pos.hpp"

static inline void Span_print(FILE* dest, struct Span span) {
    fprintf(dest, "%lu - %lu", span.start, span.end);
}
