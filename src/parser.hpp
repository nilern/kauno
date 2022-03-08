#ifndef PARSER_H
#define PARSER_H

#include "object.hpp"
#include "lexer.hpp"

namespace kauno {

static inline Handle<void> parse_expr(State& state, Lexer& lexer);

}

#endif // PARSER_H
