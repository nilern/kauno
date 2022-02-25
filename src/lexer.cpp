#include "lexer.hpp"

#include <cstdlib>
#include <cctype>

static inline void Token_print(FILE* dest, Token tok) {
    fprintf(dest, "<%s \"", TOKEN_NAMES[tok.type]);

    for (size_t i = 0; i < tok.len; ++i) {
        fputc(tok.chars[i], dest);
    }

    fputs("\" @ ", dest);

    Span_print(dest, tok.span);

    fputc('>', dest);
}

static inline Lexer Lexer_new(char const* chars, size_t len) {
    size_t const index = 0;
    return (Lexer){
        .chars = chars,
        .end = chars + len,
        .first = {
            .type = TOKEN_EOF,
            .chars = chars,
            .len = 0,
            .span = {index, index}
        },
        .index = index
    };
}

static inline void Lexer_look_ahead(Lexer* lexer) {
    while (true) {
        if (lexer->chars < lexer->end) {
            switch (*lexer->chars) {
            case '(': {
                lexer->first = (Token){
                    .type = TOKEN_LPAREN,
                    .chars = lexer->chars,
                    .len = 1,
                    .span = {lexer->index, lexer->index + 1}
                };
                return;
            }
            case ')': {
                lexer->first = (Token){
                    .type = TOKEN_RPAREN,
                    .chars = lexer->chars,
                    .len = 1,
                    .span = {lexer->index, lexer->index + 1}
                };
                return;
            }

            case ',': {
                lexer->first = (Token){
                    .type = TOKEN_COMMA,
                    .chars = lexer->chars,
                    .len = 1,
                    .span = {lexer->index, lexer->index + 1}
                };
                return;
            }

            default:
                if (isspace(*lexer->chars)) {
                    ++lexer->chars;
                    ++lexer->index;
                } else if (isalpha(*lexer->chars)) {
                    char const* const tok_chars = lexer->chars;
                    size_t const start = lexer->index;

                    size_t len = 1;
                    for (char const* chars = tok_chars + 1; chars < lexer->end && isalnum(*chars); chars++) {
                        ++len;
                    }

                    lexer->first = (Token){
                        .type = TOKEN_VAR,
                        .chars = tok_chars,
                        .len = len,
                        .span = {start, start + len - 1}
                    };
                    return;
                } else if (isdigit(*lexer->chars)) {
                    char const* const tok_chars = lexer->chars;
                    size_t const start = lexer->index;

                    size_t len = 1;
                    for (char const* chars = tok_chars + 1; chars < lexer->end && isdigit(*chars); chars++) {
                        ++len;
                    }

                    lexer->first = (Token){
                        .type = TOKEN_INT,
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
            lexer->first = (Token){
                .type = TOKEN_EOF,
                .chars = lexer->chars,
                .len = 0,
                .span = {lexer->index, lexer->index}
            };
            return;
        }
    }
}

static inline Token Lexer_peek(Lexer* lexer) {
    if (lexer->first.type == TOKEN_EOF) {
        Lexer_look_ahead(lexer);
    }

    return lexer->first;
}

static inline void Lexer_next(Lexer* lexer) {
    if (lexer->first.type == TOKEN_EOF) {
        Lexer_look_ahead(lexer);
    }

    lexer->chars = lexer->first.chars + lexer->first.len;
    lexer->index = lexer->first.span.end;
    lexer->first = (Token){
        .type = TOKEN_EOF,
        .chars = lexer->chars,
        .len = 0,
        .span = {lexer->index, lexer->index}
    };
}

static inline void Lexer_match(Lexer* lexer, TokenType type) {
    if (lexer->first.type == TOKEN_EOF) {
        Lexer_look_ahead(lexer);
    }

    if (lexer->first.type == type) {
        lexer->chars = lexer->first.chars + lexer->first.len;
        lexer->index = lexer->first.span.end;
        lexer->first = (Token){
            .type = TOKEN_EOF,
            .chars = lexer->chars,
            .len = 0,
            .span = {lexer->index, lexer->index}
        };
    } else {
        exit(EXIT_FAILURE); // FIXME
    }
}
