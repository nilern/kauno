#ifndef EVAL_H
#define EVAL_H

#include "state.hpp"
#include "locals.hpp"

static inline Handle<void> eval(State* state, ORef<void> env /* ORef<Locals | NoneType> */, ORef<void> expr);

#endif // EVAL_H
