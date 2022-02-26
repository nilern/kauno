#include "pos.hpp"

void Span::print(FILE* dest) const {
    fprintf(dest, "%lu - %lu", start, end);
}
