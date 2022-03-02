#include "parser.hpp"

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <algorithm>

#include "state.hpp"
#include "ast.hpp"

static inline Handle<void> parse_callee(State* state, Lexer* lexer);
static inline size_t parse_args(State* state, Lexer* lexer);

// expr ::= callee args?
static inline Handle<void> parse_expr(State* state, Lexer* lexer) {
    Handle<void> const callee = parse_callee(state, lexer);

    if (lexer->peek().type == Lexer::Token::Type::LPAREN) {
        size_t const argc = parse_args(state, lexer);

        Call* const call = (Call*)state->alloc_indexed(state->Call.data(), argc);
        *call = (Call){
            .callee = callee.oref(),
            .args_count = argc,
            .args = {}
        };
        ORef<void>* const args = state->peekn(argc);
        std::copy(args, args + argc, &call->args[0]);

        state->popn(argc + 1);
        return state->push(ORef(call)).template unchecked_cast<void>();
    } else {
        return callee;
    }
}

// callee ::= '(' expr ')' | VAR | INT
static inline Handle<void> parse_callee(State* state, Lexer* lexer) {
    Lexer::Token const tok = lexer->peek();

    switch (tok.type) {
    case Lexer::Token::Type::LPAREN: {
        lexer->next();
        Handle<void> const expr = parse_expr(state, lexer);
        lexer->match(Lexer::Token::Type::RPAREN);
        return expr;
    }

    case Lexer::Token::Type::VAR: {
        lexer->next();

        return Symbol_new(state, tok.chars, tok.len).template unchecked_cast<void>();
    }

    case Lexer::Token::Type::INT: {
        // FIXME: Bignums instead of overflow bugginess:
        // TODO: Return smallest possible type:

        lexer->next();

        int64_t n = 0;
        for (size_t i = 0; i < tok.len; ++i) {
            n = 10*n + (tok.chars[i] - '0');
        }

        int64_t* data = (int64_t*)state->alloc(state->Int64.data());
        *data = n;
        return state->push(ORef(data)).unchecked_cast<void>();
    }

    case Lexer::Token::Type::RPAREN:
    case Lexer::Token::Type::COMMA:
    case Lexer::Token::Type::TOKEN_EOF: {}
    }

    exit(EXIT_FAILURE); // FIXME
}

// args ::= '(' (expr (',' expr)*)? ')'
static inline size_t parse_args(State* state, Lexer* lexer) {
    size_t argc = 0;

    assert(lexer->peek().type == Lexer::Token::Type::LPAREN);
    lexer->next();

    if (lexer->peek().type != Lexer::Token::Type::RPAREN) {
        parse_expr(state, lexer);
        ++argc;

        while (lexer->peek().type != Lexer::Token::Type::RPAREN) {
            lexer->match(Lexer::Token::Type::COMMA);
            parse_expr(state, lexer);
            ++argc;
        }
    }

    lexer->next(); // discard ')'
    return argc;
}
