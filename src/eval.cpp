#include "eval.hpp"

#include <cstdlib>

#include "fn.hpp"

static inline Handle<Any> eval(State* state) {
    Handle<Any> const expr = State_peek(state);

    if (expr.type() == state->Symbol) {
        Handle<Symbol> const symbol = expr.unchecked_cast<Symbol>();

        Var* var = Globals_find(&state->globals, symbol.oref());
        if (var) {
            State_pop(state);
            return State_push(state, var->value);
        } else {
            exit(EXIT_FAILURE); // FIXME
        }
    } else if (expr.type() == state->Call) {
        Handle<Call> const call = expr.unchecked_cast<Call>();

        State_push(state, call.data()->callee);
        Handle<Any> const callee = eval(state);

        size_t const argc = call.data()->args_count;
        for (size_t i = 0; i < argc; ++i) {
            State_push(state, call.data()->args[i]);
            eval(state);
        }

        State_pop_nth(state, argc + 1); // Pop `call`

        if (callee.type() == state->Fn) {
            Handle<Fn> const fn = callee.unchecked_cast<Fn>();

            // FIXME: Check arity & arg types
            return fn.data()->code(state); // TODO: TCO
        } else {
            exit(EXIT_FAILURE); // FIXME
        }
    } else {
        return expr;
    }
}
