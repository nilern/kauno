#include "eval.hpp"

#include <cstdlib>

#include "fn.hpp"

static inline Handle<void> eval(State* state, Handle<void> env) {
    Handle<void> const expr = state->peek();

    if (expr.is_instance(state->Symbol)) {
        Handle<Symbol> const symbol = expr.unchecked_cast<Symbol>();

        if (env.is_instance(state->Locals)) {
            Handle<Locals> const locals = env.unchecked_cast<Locals>();

            std::optional<ORef<void>> const opt_value = locals.data()->find(*state, symbol.oref());
            if (opt_value.has_value()) {
                state->pop(); // `expr`
                return state->push(opt_value.value());
            }
        }

        Var* var = state->global(symbol);
        if (!var) {
            exit(EXIT_FAILURE); // FIXME
        }

        state->pop(); // `expr`
        return state->push(var->value);
    } else if (expr.is_instance(state->Call)) {
        Handle<kauno::ast::Call> const call = expr.unchecked_cast<kauno::ast::Call>();

        state->push(call.data()->callee);
        eval(state, env);

        size_t const argc = call.data()->args_count;
        for (size_t i = 0; i < argc; ++i) {
            state->push(call.data()->args[i]);
            eval(state, env);
        }

        state->pop_nth(argc + 1); // Pop `call`

        Handle<void> const callee = state->peek_nth(argc);
        if (callee.is_instance(state->Fn)) {
            Handle<kauno::fn::Fn> const fn = callee.unchecked_cast<kauno::fn::Fn>();

            if (argc != fn.data()->domain_count) {
                exit(EXIT_FAILURE); // FIXME
            }

            {
                ORef<void>* arg = state->peekn(argc);
                for (size_t i = 0; i < argc; ++i, ++arg) {
                    ORef<void> const param_ann = fn.data()->domain[i];

                    if (param_ann.is_instance(state->Type)) {
                        ORef<Type> const param_type = param_ann.unchecked_cast<Type>();

                        if (!arg->is_instance(param_type)) {
                            exit(EXIT_FAILURE); // FIXME
                        }
                    }
                }
            }

            return fn.data()->code(state); // TODO: TCO
        } else if (callee.is_instance(state->Closure)) {
            Handle<kauno::fn::Closure> const closure = callee.unchecked_cast<kauno::fn::Closure>();

            Handle<kauno::ast::Fn> const code = state->push(closure.data()->code);

            if (argc != code.data()->arity) {
                exit(EXIT_FAILURE); // FIXME
            }

            {
                ORef<kauno::arrays::RefArray<void>> const domain = code.data()->domain;

                ORef<void>* arg = state->peekn(argc + 1);
                for (size_t i = 0; i < argc; ++i, ++arg) {
                    ORef<void> const param_ann = domain.data()->elements[i];

                    if (param_ann.is_instance(state->Type)) {
                        ORef<Type> const param_type = param_ann.unchecked_cast<Type>();

                        if (!arg->is_instance(param_type)) {
                            exit(EXIT_FAILURE); // FIXME
                        }
                    }
                }
            }

            Handle<Locals> const env_ = Locals::create(*state, env, argc);

            {
                ORef<void>* arg = state->peekn(argc + 2);
                for (size_t i = 0; i < argc; ++i, ++arg) {
                    env_.data()->insert(code.data()->params[i], *arg);
                }
            }

            // TODO: TCO:
            state->push(code.data()->body);
            state->popn_nth(argc + 3, argc + 2); // `callee` & arguments & `code`
            eval(state, state->peek_nth(1));
            state->pop_nth(1); // `env_`
            return state->peek();
        } else {
            exit(EXIT_FAILURE); // FIXME
        }
    } else if (expr.is_instance(state->AstFn)) {
        Handle<kauno::ast::Fn> const fn = expr.unchecked_cast<kauno::ast::Fn>();

        kauno::fn::Closure::create(*state, fn, env);
        state->pop_nth(1); // `expr`
        return state->peek(); // the closure
    } else {
        return expr;
    }
}
