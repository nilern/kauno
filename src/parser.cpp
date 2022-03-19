#include "parser.hpp"

#include <cstdlib>
#include <cstring>
#include <cassert>

#include "ast.hpp"
#include "state.hpp"
#include "arrays.hpp"

namespace kauno {

static inline void parse_param(State& state, Lexer& lexer);
static inline AnySRef parse_call(State& state, Lexer& lexer);
static inline AnySRef parse_callee(State& state, Lexer& lexer);
static inline size_t parse_args(State& state, Lexer& lexer);

// expr ::= 'fn' '(' (param (',' param)*)? ')' '->' expr
//        | call
static inline AnySRef parse_expr(State& state, Lexer& lexer) {
    if (lexer.peek().type == Lexer::Token::Type::FN) {
        lexer.next();

        lexer.match(Lexer::Token::Type::LPAREN);

        size_t arity = 0;
        if (lexer.peek().type != Lexer::Token::Type::RPAREN) {
            parse_param(state, lexer);
            ++arity;

            while (lexer.peek().type != Lexer::Token::Type::RPAREN) {
                lexer.match(Lexer::Token::Type::COMMA);
                parse_param(state, lexer);
                ++arity;
            }
        }
        DynRef* const params = state.peekn(arity);

        lexer.next(); // discard ')'

        lexer.match(Lexer::Token::Type::ARROW);

        AnySRef const body = parse_expr(state, lexer);

        Handle<kauno::arrays::RefArray<void>> domain =
                state.push_outlined(kauno::arrays::RefArray<void>::create(state, arity));

        ORef<void> const heap_body = body.to_heap(state);
        kauno::ast::Fn* fn = static_cast<kauno::ast::Fn*>(state.alloc_indexed(state.AstFn.data(), arity));
        *fn = (kauno::ast::Fn){
            .domain = domain.oref(),
            .body = heap_body,
            .arity = arity,
            .params = {}
        };

        for (size_t i = 0; i < arity; ++i) {
            domain.data()->elements[i] = state.None.as_void();
            fn->params[i] = params[i].unchecked_oref().unchecked_cast<Symbol>();
        }

        state.popn(arity + 2); // params & body & domain
        return state.push(ORef(fn).as_void());
    } else {
        return parse_call(state, lexer);
    }
}

// call ::= callee args?
static inline AnySRef parse_call(State& state, Lexer& lexer) {
    AnySRef const callee = parse_callee(state, lexer);

    if (lexer.peek().type == Lexer::Token::Type::LPAREN) {
        size_t const argc = parse_args(state, lexer);

        Handle<void> const heap_callee = state.push_outlined(callee.to_heap(state));
        Handle<kauno::ast::Call> const call =
                state.push_outlined(ORef(static_cast<kauno::ast::Call*>(state.alloc_indexed(state.Call.data(), argc))));
        *call.data() = (kauno::ast::Call){
            .callee = heap_callee.oref(),
            .args_count = argc,
            .args = {}
        };
        DynRef* const args = state.peekn(argc + 2);
        for (size_t i = 0; i < argc; ++i) {
            call.data()->args[i] = args[i].to_heap(state);
        }

        state.popn_nth(argc + 2, argc + 2); // callee & args & callee
        return state.peek(); // call as `AnySRef`
    } else {
        return callee;
    }
}

// param ::= VAR
static inline void parse_param(State& state, Lexer& lexer) {
    Lexer::Token const tok = lexer.peek();

    if (tok.type == Lexer::Token::Type::VAR) {
        lexer.next();
        Symbol_new(state, tok.chars, tok.len).template unchecked_cast<void>();
    } else {
        exit(EXIT_FAILURE); // FIXME
    }
}

// callee ::= '(' expr ')' | VAR | INT
static inline AnySRef parse_callee(State& state, Lexer& lexer) {
    Lexer::Token const tok = lexer.peek();

    switch (tok.type) {
    case Lexer::Token::Type::LPAREN: {
        lexer.next();
        AnySRef const expr = parse_expr(state, lexer);
        lexer.match(Lexer::Token::Type::RPAREN);
        return expr;
    }

    case Lexer::Token::Type::VAR: {
        lexer.next();

        Symbol_new(state, tok.chars, tok.len);
        return state.peek(); // symbol as `AnySRef`
    }

    case Lexer::Token::Type::INT: {
        // FIXME: Bignums instead of overflow bugginess:
        // TODO: Return smallest possible type:

        lexer.next();

        int64_t n = 0;
        for (size_t i = 0; i < tok.len; ++i) {
            n = 10*n + (tok.chars[i] - '0');
        }

        SRef<int64_t> const data = state.stack_alloc<int64_t>(state.Int64);
        *data.data() = n;
        return state.peek(); // The int as `AnySRef`
    }

    case Lexer::Token::Type::ARROW:
    case Lexer::Token::Type::FN:
    case Lexer::Token::Type::RPAREN:
    case Lexer::Token::Type::COMMA:
    case Lexer::Token::Type::TOKEN_EOF: {}
    }

    exit(EXIT_FAILURE); // FIXME
}

// args ::= '(' (expr (',' expr)*)? ')'
static inline size_t parse_args(State& state, Lexer& lexer) {
    size_t argc = 0;

    assert(lexer.peek().type == Lexer::Token::Type::LPAREN);
    lexer.next();

    if (lexer.peek().type != Lexer::Token::Type::RPAREN) {
        parse_expr(state, lexer);
        ++argc;

        while (lexer.peek().type != Lexer::Token::Type::RPAREN) {
            lexer.match(Lexer::Token::Type::COMMA);
            parse_expr(state, lexer);
            ++argc;
        }
    }

    lexer.next(); // discard ')'
    return argc;
}

}
