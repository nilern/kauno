#ifndef LEXER_H
#define LEXER_H

#include <cstddef>
#include <cstdio>

#include "pos.hpp"

class Lexer {
public:
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

        void print(FILE* dest) const;
    };

private:
    char const* chars;
    char const* end;
    Token first;
    size_t index;

public:
    Lexer(char const* chars_, size_t len_) :
        chars(chars_),
        end(chars_ + len_)
    {
        index = 0;
        first = {
            .type = Lexer::Token::Type::TOKEN_EOF,
            .chars = chars,
            .len = 0,
            .span = {index, index}
        };
    }

    Lexer::Token peek();

    void next();

    void match(Lexer::Token::Type type);

private:
    void look_ahead();
};

#endif // LEXER_H
