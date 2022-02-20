#include "eval.hpp"

#include <cstdlib>

static inline Handle<Any> eval(State* state) {
    Handle<Any> const expr = State_peek(state);

    if (expr.type() == state->Symbol) {
        Handle<Symbol> symbol = expr.unchecked_cast<Symbol>();

        Var* var = Globals_find(&state->globals, symbol.oref());
        if (var) {
            return State_push(state, var->value);
        } else {
            exit(EXIT_FAILURE); // FIXME
        }
    } else {
        return expr;
    }
}
