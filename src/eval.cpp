#include "eval.hpp"

#include <cstdlib>

#include "fn.hpp"

namespace kauno {

static inline Handle<void> eval(State* state, ORef<void> oenv, ORef<void> oexpr) {
    while (true) {
        if (oexpr.is_instance<Symbol>(*state)) {
            ORef<Symbol> const symbol = oexpr.unchecked_cast<Symbol>();

            if (oenv.is_instance<Locals>(*state)) {
                ORef<Locals> const locals = oenv.unchecked_cast<Locals>();

                std::optional<ORef<void>> const opt_value = locals.data()->find(*state, symbol);
                if (opt_value.has_value()) {
                    return state->push(opt_value.value());
                }
            }

            Var* var = state->global(symbol);
            if (!var) {
                exit(EXIT_FAILURE); // FIXME
            }

            return state->push(var->value);
        } else if (oexpr.is_instance<ast::Call>(*state)) {
            Handle<void> const env = state->push(oenv);
            Handle<ast::Call> const call = state->push(oexpr.unchecked_cast<ast::Call>());

            eval(state, env.oref(), call.data()->callee);

            size_t const argc = call.data()->args_count;
            for (size_t i = 0; i < argc; ++i) {
                eval(state, env.oref(), call.data()->args[i]);
            }

            state->pop_nth(1 + argc); // Pop `call`

            Handle<void> const callee = state->peek_nth(argc);
            if (callee.is_instance<fn::Fn>(*state)) {
                Handle<fn::Fn> fn = callee.unchecked_cast<fn::Fn>();

                if (argc != fn.data()->domain_count) {
                    exit(EXIT_FAILURE); // FIXME
                }

                {
                    ORef<void>* arg = state->peekn(argc);
                    for (size_t i = 0; i < argc; ++i, ++arg) {
                        ORef<void> const param_ann = fn.data()->domain[i];

                        if (param_ann.is_instance<Type>(*state)) {
                            ORef<Type> const param_type = param_ann.unchecked_cast<Type>();

                            if (!arg->is_instance_dyn(param_type)) {
                                exit(EXIT_FAILURE); // FIXME
                            }
                        }
                    }
                }

                state->pop_nth(1 + argc); // env
                fn = state->peek_nth(argc).unchecked_cast<fn::Fn>();
                return fn.data()->code(state);
            } else if (callee.is_instance<fn::Closure>(*state)) {
                Handle<fn::Closure> const closure = callee.unchecked_cast<fn::Closure>();

                Handle<ast::Fn> const code = state->push(closure.data()->code);

                if (argc != code.data()->arity) {
                    exit(EXIT_FAILURE); // FIXME
                }

                {
                    ORef<arrays::RefArray<void>> const domain = code.data()->domain;

                    ORef<void>* arg = state->peekn(argc + 1);
                    for (size_t i = 0; i < argc; ++i, ++arg) {
                        ORef<void> const param_ann = domain.data()->elements[i];

                        if (param_ann.is_instance<Type>(*state)) {
                            ORef<Type> const param_type = param_ann.unchecked_cast<Type>();

                            if (!arg->is_instance_dyn(param_type)) {
                                exit(EXIT_FAILURE); // FIXME
                            }
                        }
                    }
                }

                Handle<void> const closure_env = state->push(closure.data()->env);
                ORef<Locals> const oenv_ = Locals::create(*state, closure_env, argc).oref();

                {
                    ORef<void>* arg = state->peekn(argc + 3);
                    for (size_t i = 0; i < argc; ++i, ++arg) {
                        oenv_.data()->insert(code.data()->params[i], *arg);
                    }
                }

                // TCO'd `return eval(state, oenv_.as_void(), code.data()->body);`:
                oenv = oenv_.as_void();
                oexpr = code.data()->body;
                state->popn(argc + 5); // env & `callee` & args & `code` & `closure_env` & env_
            } else {
                exit(EXIT_FAILURE); // FIXME
            }
        } else if (oexpr.is_instance<ast::Fn>(*state)) {
            Handle<void> const env = state->push(oenv);
            Handle<ast::Fn> const fn = state->push(oexpr.unchecked_cast<ast::Fn>());

            fn::Closure::create(*state, fn, env);
            state->popn_nth(2, 2); // `env` & `fn`
            return state->peek(); // the closure
        } else {
            return state->push(oexpr);
        }
    }
}

}
