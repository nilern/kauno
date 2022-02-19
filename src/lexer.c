#include "lexer.h"

#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

static inline void Token_print(FILE* dest, struct Token tok) {
    fprintf(dest, "<%s \"", TOKEN_NAMES[tok.type]);

    for (size_t i = 0; i < tok.len; ++i) {
        fputc(tok.chars[i], dest);
    }

    fputs("\" @ ", dest);

    Span_print(dest, tok.span);

    fputc('>', dest);
}

static inline struct Lexer Lexer_new(char const* chars, size_t len) {
    size_t const index = 0;
    return (struct Lexer){
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

static inline void Lexer_look_ahead(struct Lexer* lexer) {
    while (true) {
        if (lexer->chars < lexer->end) {
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

                lexer->first = (struct Token){
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

                lexer->first = (struct Token){
                    .type = TOKEN_INT,
                    .chars = tok_chars,
                    .len = len,
                    .span = {start, start + len - 1}
                };
                return;
            } else {
                exit(EXIT_FAILURE); // FIXME
            }
        } else {
            lexer->first = (struct Token){
                .type = TOKEN_EOF,
                .chars = lexer->chars,
                .len = 0,
                .span = {lexer->index, lexer->index}
            };
            return;
        }
    }
}

static inline struct Token Lexer_peek(struct Lexer* lexer) {
    if (lexer->first.type == TOKEN_EOF) {
        Lexer_look_ahead(lexer);
    }

    return lexer->first;
}

static inline void Lexer_next(struct Lexer* lexer) {
    if (lexer->first.type == TOKEN_EOF) {
        Lexer_look_ahead(lexer);
    }

    lexer->chars = lexer->first.chars + lexer->first.len;
    lexer->index = lexer->first.span.end;
    lexer->first = (struct Token){
        .type = TOKEN_EOF,
        .chars = lexer->chars,
        .len = 0,
        .span = {lexer->index, lexer->index}
    };
}
