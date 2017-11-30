#pragma once
#include <optional>

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
struct store : std::tuple<TStates...> {
  using T = std::tuple<TStates...>;

  constexpr store(TReducer reducer, TStates &&... t)
      : std::tuple<TStates...>{std::forward<TStates>(t)...}, reduce{reducer} {}

  const T &state() const { return *static_cast<const T *>(this); }

  void dispatch(const auto &action) {
    *static_cast<T *>(this) = reduce(state(), action);
  }

  const TReducer reduce;
};
} // namespace redux
