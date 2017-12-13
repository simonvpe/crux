#pragma once
#include "../redux.hpp"

#include <future>

namespace redux {

const auto thunk = redux::make_middleware(
    [](const auto &state, auto &&dispatch, auto &&next, const auto &action) {
      using action_t = decltype(action);
      using dispatch_t = decltype(dispatch);
      using state_t = decltype(state);
      if constexpr (std::is_invocable_v<action_t, dispatch_t, state_t>) {
        return action(std::forward<dispatch_t>(dispatch), state);
      } else {
        return next(action);
      }
    });
}
