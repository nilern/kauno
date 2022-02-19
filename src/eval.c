#include "eval.h"

#include <stdlib.h>

static inline Handle eval(struct State* state) {
    Handle const expr = State_peek(state);
    ORef const expr_oref = Handle_oref(expr);

    if (obj_type(expr_oref) == state->Symbol) {
        struct Symbol* symbol = (struct Symbol*)obj_data(expr_oref);

        struct Var* var = Globals_find(&state->globals, symbol);
        if (var) {
            return State_push(state, var->value);
        } else {
            exit(EXIT_FAILURE); // FIXME
        }
    } else {
        return expr;
    }
}
