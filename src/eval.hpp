#ifndef EVAL_H
#define EVAL_H

#include "state.hpp"
#include "locals.hpp"

static inline Handle<void> eval(State* state, Handle<void> env /* Handle<Locals | NoneType> */);

#endif // EVAL_H
