#ifndef PARSER_H
#define PARSER_H

#include "object.hpp"
#include "lexer.hpp"

static inline Handle<Any> parse_expr(State* state, Lexer* lexer);

#endif // PARSER_H
