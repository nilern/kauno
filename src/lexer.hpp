#ifndef LEXER_H
#define LEXER_H

#include <cstddef>
#include <cstdio>

#include "pos.hpp"

enum TokenType {
    TOKEN_VAR,
    TOKEN_INT,
    TOKEN_EOF
};

static char const* const TOKEN_NAMES[TOKEN_EOF + 1] = {
    "VAR",
    "INT",
    "EOF"
};

struct Token {
    enum TokenType type;
    char const* chars;
    size_t len;
    Span span;
};

struct Lexer {
    char const* chars;
    char const* end;
    Token first;
    size_t index;
};

static inline void Token_print(FILE* dest, Token tok);

static inline Lexer Lexer_new(char const* chars, size_t len);

static inline Token Lexer_peek(Lexer* lexer);

static inline void Lexer_next(Lexer* lexer);

#endif // LEXER_H
