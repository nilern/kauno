#include "lexer.hpp"

#include <cstdlib>
#include <cctype>

void Lexer::Token::print(FILE* dest) const {
    fprintf(dest, "<%s \"", Lexer::Token::NAMES[(size_t)type]);

    for (size_t i = 0; i < len; ++i) {
        fputc(chars[i], dest);
    }

    fputs("\" @ ", dest);

    Span_print(dest, span);

    fputc('>', dest);
}

void Lexer::look_ahead() {
    while (true) {
        if (chars < end) {
            switch (*chars) {
            case '(': {
                first = (Lexer::Token){
                    .type = Lexer::Token::Type::LPAREN,
                    .chars = chars,
                    .len = 1,
                    .span = {index, index + 1}
                };
                return;
            }
            case ')': {
                first = (Lexer::Token){
                    .type = Lexer::Token::Type::RPAREN,
                    .chars = chars,
                    .len = 1,
                    .span = {index, index + 1}
                };
                return;
            }

            case ',': {
                first = (Lexer::Token){
                    .type = Lexer::Token::Type::COMMA,
                    .chars = chars,
                    .len = 1,
                    .span = {index, index + 1}
                };
                return;
            }

            default:
                if (isspace(*chars)) {
                    ++chars;
                    ++index;
                } else if (isalpha(*chars)) {
                    char const* const tok_chars = chars;
                    size_t const start = index;

                    size_t len = 1;
                    for (char const* chars = tok_chars + 1; chars < end && isalnum(*chars); chars++) {
                        ++len;
                    }

                    first = (Lexer::Token){
                        .type = Lexer::Token::Type::VAR,
                        .chars = tok_chars,
                        .len = len,
                        .span = {start, start + len - 1}
                    };
                    return;
                } else if (isdigit(*chars)) {
                    char const* const tok_chars = chars;
                    size_t const start = index;

                    size_t len = 1;
                    for (char const* chars = tok_chars + 1; chars < end && isdigit(*chars); chars++) {
                        ++len;
                    }

                    first = (Lexer::Token){
                        .type = Lexer::Token::Type::INT,
                        .chars = tok_chars,
                        .len = len,
                        .span = {start, start + len - 1}
                    };
                    return;
                } else {
                    exit(EXIT_FAILURE); // FIXME
                }
            }
        } else {
            first = (Lexer::Token){
                .type = Lexer::Token::Type::TOKEN_EOF,
                .chars = chars,
                .len = 0,
                .span = {index, index}
            };
            return;
        }
    }
}

Lexer::Token Lexer::peek() {
    if (first.type == Lexer::Token::Type::TOKEN_EOF) {
        look_ahead();
    }

    return first;
}

void Lexer::next() {
    if (first.type == Lexer::Token::Type::TOKEN_EOF) {
        look_ahead();
    }

    chars = first.chars + first.len;
    index = first.span.end;
    first = (Lexer::Token){
        .type = Lexer::Token::Type::TOKEN_EOF,
        .chars = chars,
        .len = 0,
        .span = {index, index}
    };
}

void Lexer::match(Lexer::Token::Type type) {
    if (first.type == Lexer::Token::Type::TOKEN_EOF) {
        look_ahead();
    }

    if (first.type == type) {
        chars = first.chars + first.len;
        index = first.span.end;
        first = (Lexer::Token){
            .type = Lexer::Token::Type::TOKEN_EOF,
            .chars = chars,
            .len = 0,
            .span = {index, index}
        };
    } else {
        exit(EXIT_FAILURE); // FIXME
    }
}
