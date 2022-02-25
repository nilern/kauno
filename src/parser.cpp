#include "parser.hpp"

#include <cstdlib>
#include <cstring>
#include <cassert>

#include "state.hpp"
#include "ast.hpp"

static inline Handle<Any> parse_callee(State* state, Lexer* lexer);
static inline size_t parse_args(State* state, Lexer* lexer);

static inline Handle<Any> parse_expr(State* state, Lexer* lexer) {
    Handle<Any> const callee = parse_callee(state, lexer);

    if (Lexer_peek(lexer).type == TOKEN_LPAREN) {
        size_t const argc = parse_args(state, lexer);

        Call* const call = (Call*)state->heap.alloc_indexed(state->Call.data(), argc);
        *call = (Call){
            .callee = callee.oref(),
            .args_count = argc,
            .args = {}
        };
        if (argc > 0) {
            memcpy(&call->args, State_peek_nth(state, argc - 1).oref_ptr(), sizeof(ORef<Any>)*argc);
        }

        State_popn(state, argc + 1);
        return State_push(state, ORef(call)).template unchecked_cast<Any>();
    } else {
        return callee;
    }
}

// callee ::= '(' expr ')' | VAR | INT
static inline Handle<Any> parse_callee(State* state, Lexer* lexer) {
    Token const tok = Lexer_peek(lexer);

    switch (tok.type) {
    case TOKEN_LPAREN: {
        Lexer_next(lexer);
        Handle<Any> const expr = parse_expr(state, lexer);
        Lexer_match(lexer, TOKEN_RPAREN);
        return expr;
    }

    case TOKEN_VAR: {
        Lexer_next(lexer);

        return Symbol_new(state, tok.chars, tok.len).template unchecked_cast<Any>();
    }

    case TOKEN_INT: {
        // FIXME: Bignums instead of overflow bugginess:
        // TODO: Return smallest possible type:

        Lexer_next(lexer);

        int64_t n = 0;
        for (size_t i = 0; i < tok.len; ++i) {
            n = 10*n + (tok.chars[i] - '0');
        }

        int64_t* data = (int64_t*)state->heap.alloc(state->Int64.data());
        *data = n;
        return State_push(state, ORef(data)).template unchecked_cast<Any>();
    }

    case TOKEN_RPAREN:
    case TOKEN_COMMA:
    case TOKEN_EOF: {}
    }

    exit(EXIT_FAILURE); // FIXME
}

// args ::= '(' (expr (',' expr)*)? ')'
static inline size_t parse_args(State* state, Lexer* lexer) {
    size_t argc = 0;

    assert(Lexer_peek(lexer).type == TOKEN_LPAREN);
    Lexer_next(lexer);

    if (Lexer_peek(lexer).type != TOKEN_RPAREN) {
        parse_expr(state, lexer);
        ++argc;

        while (Lexer_peek(lexer).type != TOKEN_RPAREN) {
            Lexer_match(lexer, TOKEN_COMMA);
            parse_expr(state, lexer);
            ++argc;
        }
    }

    Lexer_next(lexer); // discard ')'
    return argc;
}
