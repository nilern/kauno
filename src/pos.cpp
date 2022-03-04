#include "pos.hpp"

namespace kauno {

void Span::print(FILE* dest) const {
    fprintf(dest, "%lu - %lu", start, end);
}

}
