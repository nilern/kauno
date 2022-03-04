#include "eval.hpp"

#include <cstdlib>

#include "fn.hpp"

namespace kauno {

static inline Handle<void> eval(State* state, ORef<void> oenv, ORef<void> oexpr) {
    while (true) {
        optional<ORef<Symbol>> const opt_symbol = oexpr.try_cast<Symbol>(*state);
        if (opt_symbol.has_value()) {
            ORef<Symbol> const symbol = *opt_symbol;

            optional<ORef<Locals>> opt_locals = oenv.try_cast<Locals>(*state);
            if (opt_locals.has_value()) {
                ORef<Locals> const locals = *opt_locals;

                std::optional<ORef<void>> const opt_value = locals.data()->find(*state, symbol);
                if (opt_value.has_value()) {
                    return state->push(*opt_value);
                }
            }

            Var* var = state->global(symbol);
            if (!var) {
                exit(EXIT_FAILURE); // FIXME
            }

            return state->push(var->value);
        }

        optional<ORef<ast::Call>> const opt_call = oexpr.try_cast<ast::Call>(*state);
        if (opt_call.has_value()) {
            Handle<void> const env = state->push(oenv);
            Handle<ast::Call> const call = state->push(*opt_call);

            eval(state, env.oref(), call.data()->callee);

            size_t const argc = call.data()->args_count;
            for (size_t i = 0; i < argc; ++i) {
                eval(state, env.oref(), call.data()->args[i]);
            }

            state->pop_nth(1 + argc); // Pop `call`

            Handle<void> const callee = state->peek_nth(argc);

            optional<Handle<fn::Fn>> const opt_fn = callee.try_cast<fn::Fn>(*state);
            if (opt_fn.has_value()) {
                Handle<fn::Fn> fn = *opt_fn;

                if (argc != fn.data()->domain_count) {
                    exit(EXIT_FAILURE); // FIXME
                }

                {
                    ORef<void>* arg = state->peekn(argc);
                    for (size_t i = 0; i < argc; ++i, ++arg) {
                        ORef<void> const param_ann = fn.data()->domain[i];

                        optional<ORef<Type>> opt_param_type = param_ann.try_cast<Type>(*state);
                        if (opt_param_type.has_value()) {
                            ORef<Type> const param_type = *opt_param_type;

                            if (!arg->is_instance_dyn(param_type)) {
                                exit(EXIT_FAILURE); // FIXME
                            }
                        }
                    }
                }

                state->pop_nth(1 + argc); // env
                fn = state->peek_nth(argc).unchecked_cast<fn::Fn>();
                return fn.data()->code(state);
            }

            optional<Handle<fn::Closure>> const opt_closure = callee.try_cast<fn::Closure>(*state);
            if (opt_closure.has_value()) {
                Handle<fn::Closure> const closure = *opt_closure;

                Handle<ast::Fn> const code = state->push(closure.data()->code);

                if (argc != code.data()->arity) {
                    exit(EXIT_FAILURE); // FIXME
                }

                {
                    ORef<arrays::RefArray<void>> const domain = code.data()->domain;

                    ORef<void>* arg = state->peekn(argc + 1);
                    for (size_t i = 0; i < argc; ++i, ++arg) {
                        ORef<void> const param_ann = domain.data()->elements[i];

                        optional<ORef<Type>> opt_param_type = param_ann.try_cast<Type>(*state);
                        if (opt_param_type.has_value()) {
                            ORef<Type> const param_type = *opt_param_type;

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
                continue;
            }

            exit(EXIT_FAILURE); // FIXME
        }

        optional<ORef<ast::Fn>> const opt_fn = oexpr.try_cast<ast::Fn>(*state);
        if (opt_fn.has_value()) {
            Handle<void> const env = state->push(oenv);
            Handle<ast::Fn> const fn = state->push(*opt_fn);

            fn::Closure::create(*state, fn, env);
            state->popn_nth(2, 2); // `env` & `fn`
            return state->peek(); // the closure
        }

        return state->push(oexpr);
    }
}

}
