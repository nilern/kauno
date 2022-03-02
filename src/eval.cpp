#include "eval.hpp"

#include <cstdlib>

#include "fn.hpp"

static inline Handle<Any> eval(State* state) {
    Handle<Any> const expr = state->peek();

    if (expr.type() == state->Symbol) {
        Handle<Symbol> const symbol = expr.unchecked_cast<Symbol>();

        Var* var = state->global(symbol);
        if (!var) {
            exit(EXIT_FAILURE); // FIXME
        }

        state->pop();
        return state->push(var->value);
    } else if (expr.type() == state->Call) {
        Handle<Call> const call = expr.unchecked_cast<Call>();

        state->push(call.data()->callee);
        eval(state);

        size_t const argc = call.data()->args_count;
        for (size_t i = 0; i < argc; ++i) {
            state->push(call.data()->args[i]);
            eval(state);
        }

        state->pop_nth(argc + 1); // Pop `call`

        Handle<Any> const callee = state->peek_nth(argc);
        if (callee.type() == state->Fn) {
            Handle<kauno::fn::Fn> const fn = callee.unchecked_cast<kauno::fn::Fn>();

            if (argc != fn.data()->domain_count) {
                exit(EXIT_FAILURE); // FIXME
            }

            {
                ORef<Any>* arg = state->peekn(argc);
                for (size_t i = 0; i < argc; ++i, ++arg) {
                    ORef<Any> const param_ann = fn.data()->domain[i];

                    if (param_ann.is_instance(state->Type)) {
                        ORef<Type> const param_type = param_ann.unchecked_cast<Type>();

                        if (!arg->is_instance(param_type)) {
                            exit(EXIT_FAILURE); // FIXME
                        }
                    }
                }
            }

            return fn.data()->code(state); // TODO: TCO
        } else {
            exit(EXIT_FAILURE); // FIXME
        }
    } else {
        return expr;
    }
}
