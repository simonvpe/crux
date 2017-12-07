#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

namespace redux::detail {
template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;
auto operator|(auto &&fleft, auto &&fright) { return fleft(fright); }
} // namespace redux::detail

namespace redux {
auto combine_reducers(auto &&... fs) {
  const auto dispatch =
      detail::overloaded{fs..., [](auto &&s, auto &&) { return s; }};
  return [&](const auto &state, const auto &action) {
    return std::apply(
        [&](const auto &... s) {
          return std::make_tuple(dispatch(s, action)...);
        },
        state);
  };
}

constexpr auto make_middleware(auto &&f) {
  return [&](const auto &store, auto &&dispatch) {
    return [&](auto &&next) {
      return [&](const auto &action) {
        f(store, dispatch, std::forward<decltype(next)>(next), action);
      };
    };
  };
}

template <typename TReducer, typename TState, typename... TMiddlewares>
class store {
  using TSubscriberId = int;
  using TSubscriberFun = std::function<void()>;
  using TSubscriber = std::pair<TSubscriberId, TSubscriberFun>;
  using TMiddleware = std::tuple<TMiddlewares...>;

public:
  constexpr store(TReducer reducer, TState &&initial_state, TMiddlewares... mw)
      : data{std::forward<TState>(initial_state)}, reduce{reducer},
        middleware{mw...} {}

  const TState &state() const { return data; }

  void dispatch(const auto &action) {
    const auto store_dispatch = [this](const auto &action) {
      data = reduce(state(), action);
    };

    const auto mw_dispatch = [this](const auto &action) { dispatch(action); };

    const auto apply_middleware_and_reducer = std::apply(
        [this, &store_dispatch, &mw_dispatch](auto &&... mw) {
          using ::redux::detail::operator|;

          return (mw(state(), mw_dispatch) | ... | store_dispatch);
        },
        middleware);

    apply_middleware_and_reducer(action);

    for (const auto &sub : subscribers)
      sub.second();
  }

  auto subscribe(TSubscriberFun &&f) {
    const auto id = next_subscriber_id++;
    subscribers.push_back({id, std::forward<TSubscriberFun>(f)});
    return [id, this] {
      const auto idx =
          std::find_if(cbegin(subscribers), cend(subscribers),
                       [id](const auto &sub) { return sub.first == id; });
      if (idx != cend(subscribers)) {
        subscribers.erase(idx);
      }
    };
  }

private:
  TState data;
  const TReducer reduce;
  const TMiddleware middleware;
  std::vector<TSubscriber> subscribers = {};
  TSubscriberId next_subscriber_id = 0;
};
} // namespace redux
