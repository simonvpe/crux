#pragma once

#include "action.hpp"
#include "state.hpp"

#include <redux.hpp>

const auto reduce = redux::combine_reducers(
    [](const A &state, const count_up &action) {
      return A{state.value + action.value, state.copied + 1};
    },
    [](const A &state, const count_down &action) {
      return A{state.value - action.value, state.copied + 1};
    },
    [](const B &state, const multiply &action) {
      return B{state.value * action.value, state.copied + 1};
    },
    [](const B &state, const divide &action) {
      throw "Divide";
      return B{state.value / action.value, state.copied + 1};
    });
