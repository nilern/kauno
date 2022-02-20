#include "pos.hpp"

static inline void Span_print(FILE* dest, Span span) {
    fprintf(dest, "%lu - %lu", span.start, span.end);
}
