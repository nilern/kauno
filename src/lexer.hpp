#ifndef LEXER_H
#define LEXER_H

#include <cstddef>
#include <cstdio>

#include "pos.hpp"

struct Lexer {
    struct Token {
        enum class Type {
            LPAREN, RPAREN,
            COMMA,
            VAR,
            INT,
            TOKEN_EOF
        };

        static constexpr char const* const NAMES[6] = {
            "LPAREN", "RPAREN",
            "COMMA",
            "VAR",
            "INT",
            "EOF"
        };

        enum Type type;
        char const* chars;
        size_t len;
        Span span;
    };

    char const* chars;
    char const* end;
    Token first;
    size_t index;
};

static inline void Token_print(FILE* dest, Lexer::Token tok);

static inline Lexer Lexer_new(char const* chars, size_t len);

static inline Lexer::Token Lexer_peek(Lexer* lexer);

static inline void Lexer_next(Lexer* lexer);

static inline void Lexer_match(Lexer* lexer, Lexer::Token::Type type);

#endif // LEXER_H
