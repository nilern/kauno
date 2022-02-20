#include "parser.hpp"

#include <cstdlib>

#include "state.hpp"

static inline void parse_expr(State* state, Lexer* lexer) {
    Token const tok = Lexer_peek(lexer);

    switch (tok.type) {
    case TOKEN_VAR: {
        Lexer_next(lexer);

        Symbol_new(state, tok.chars, tok.len);
        return;
    }

    case TOKEN_INT: {
        // FIXME: Bignums instead of overflow bugginess:
        // TODO: Return smallest possible type:

        Lexer_next(lexer);

        int64_t n = 0;
        for (size_t i = 0; i < tok.len; ++i) {
            n = 10*n + (tok.chars[i] - '0');
        }

        int64_t* data = (int64_t*)state->heap.alloc(state->Int64);
        *data = n;
        State_push(state, oref_from_ptr(data));
        return;
    }

    case TOKEN_EOF: exit(EXIT_FAILURE); // FIXME
    }
}
