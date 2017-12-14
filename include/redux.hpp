#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

namespace redux::detail {

template <typename... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
template <typename... Ts> overloaded(Ts...)->overloaded<Ts...>;

template <typename T> auto chain_middleware(T &&f) { return f; }

template <typename T, typename... U>
auto chain_middleware(T &&f, U &&... rest) {
  return f(chain_middleware(rest...));
}

} // namespace redux::detail

namespace redux {

template <typename... T> auto combine_reducers(T &&... fs) {
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

template <typename T> constexpr auto make_middleware(T &&f) {
  return [=](const auto &store, auto &&dispatch) {
    return [=](auto next) {
      return
          [=](const auto &action) { return f(store, dispatch, next, action); };
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

  template <typename T> auto dispatch(const T &action) {
    auto store_dispatch = [=](const auto &action) {
      data = reduce(data, action);
    };

    auto mw_dispatch = [=](const auto &action) { dispatch(action); };

    auto all_dispatch = [=](const auto &action) {
      return std::apply(
          [=](auto &&... mw) {
            return detail::chain_middleware(mw(this->data, mw_dispatch)...,
                                            store_dispatch);
          },
          middleware)(action);
    };

    if constexpr (std::is_same_v<decltype(all_dispatch(action)), void>) {
      all_dispatch(action);
      for (const auto &sub : subscribers)
        sub.second();
    } else {
      auto result = all_dispatch(action);
      for (const auto &sub : subscribers)
        sub.second();
      return result;
    }
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
