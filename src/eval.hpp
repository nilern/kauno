#ifndef EVAL_H
#define EVAL_H

#include "state.hpp"
#include "locals.hpp"

namespace kauno {

static inline AnySRef eval(State& state, ORef<void> env /* ORef<Locals | NoneType> */, ORef<void> expr);

}

#endif // EVAL_H
