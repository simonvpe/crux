#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

namespace redux::detail {
template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;
template <typename T, typename... Ts> constexpr bool contains() {
  return std::disjunction_v<std::is_same<T, Ts>...>;
}
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

template <typename TReducer, typename... TStates>
class store : std::tuple<TStates...> {
  using T = std::tuple<TStates...>;
  using TSubscriberId = int;
  using TSubscriberFun = std::function<void()>;
  using TSubscriber = std::pair<TSubscriberId, TSubscriberFun>;

public:
  constexpr store(TReducer reducer, std::tuple<TStates...> &&initial_state)
      : std::tuple<TStates...>{std::forward<std::tuple<TStates...>>(
            initial_state)},
        reduce{reducer} {}

  const T &state() const { return *static_cast<const T *>(this); }

  void dispatch(const auto &action) {
    *static_cast<T *>(this) = reduce(state(), action);
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
  const TReducer reduce;
  std::vector<TSubscriber> subscribers = {};
  TSubscriberId next_subscriber_id = 0;
};
} // namespace redux
