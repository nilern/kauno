#include "eval.hpp"

#include <cstdlib>

static inline Handle eval(State* state) {
    Handle const expr = State_peek(state);
    ORef const expr_oref = Handle_oref(expr);

    if (obj_type(expr_oref) == state->Symbol) {
        Symbol* symbol = (Symbol*)obj_data(expr_oref);

        Var* var = Globals_find(&state->globals, symbol);
        if (var) {
            return State_push(state, var->value);
        } else {
            exit(EXIT_FAILURE); // FIXME
        }
    } else {
        return expr;
    }
}
