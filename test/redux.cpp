#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <redux.hpp>

#include <tuple>
#include <type_traits>
#include <variant>

struct A {
  int value = 0;
  int copied = 0;
  A(int v) : value{v} {}
  A(A &&) = default;
  A(const A &other) {
    value = other.value;
    copied = other.copied + 1;
  }
  A &operator=(const A &other) {
    value = other.value;
    copied = other.copied + 1;
    return *this;
  }
  A &operator=(A &&) = default;
};

struct B {
  int value = 0;
  int copied = 0;
  B(int v) : value{v} {}
  B(B &&) = default;
  B(const B &other) {
    value = other.value;
    copied = other.copied + 1;
  }
  B &operator=(const B &other) {
    value = other.value;
    copied = other.copied + 1;
    if (copied == 2)
      throw "Too much";
    return *this;
  }
  B &operator=(B &&) = default;
};

struct count_up {
  const int value = 0;
};

struct count_down {
  const int value = 0;
};

struct multiply {
  const int value = 0;
};

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
    const auto f = [&](const auto &s) { return dispatch(s, action); };
    const auto g = [&](const auto &... s) { return std::make_tuple(f(s)...); };
    return std::apply(g, state);
  };
}

template <typename TReducer, typename... TStates>
struct store : std::tuple<TStates...> {
  using T = std::tuple<TStates...>;

  constexpr store(TReducer reducer, TStates &&... t)
      : std::tuple<TStates...>{std::forward<TStates>(t)...}, reduce{reducer} {}

  const T &state() const { return *static_cast<const T *>(this); }

  void dispatch(const auto &action) {
    const auto copies = std::get<A>(*this).copied;
    *static_cast<T *>(this) = reduce(state(), action);
    CHECK(copies + 1 == std::get<A>(*this).copied);
  }

  const TReducer reduce;
};
} // namespace redux

const auto reduce = redux::combine_reducers(
    [](const A &state, const count_up &action) {
      auto next_state = state;
      next_state.value += action.value;
      return next_state;
    },
    [](const A &state, const count_down &action) {
      auto next_state = state;
      next_state.value -= action.value;
      return next_state;
    },
    [](const B &state, const multiply &action) {
      auto next_state = state;
      next_state.value *= action.value;
      return next_state;
    });

SCENARIO("Store") {
  auto store = redux::store{reduce, A{0}, B{7}};
  CHECK(std::get<A>(store.state()).value == 0);

  store.dispatch(count_up{5});
  CHECK(std::get<A>(store.state()).value == 5);
  CHECK(std::get<A>(store.state()).copied == 1);
}

SCENARIO("Reduce") {

  auto s0 = std::make_tuple<A, B>(0, 5);
  CHECK(std::get<A>(s0).copied == 0);
  CHECK(std::get<B>(s0).copied == 0);

  auto s1 = reduce(s0, count_up{3});
  CHECK(std::get<A>(s1).value == 3);
  CHECK(std::get<A>(s1).copied == 1);
  CHECK(std::get<B>(s1).copied == 1);

  auto s2 = reduce(s1, count_up{3});
  CHECK(std::get<A>(s2).value == 6);
  CHECK(std::get<A>(s2).copied == 2);
  CHECK(std::get<B>(s2).copied == 2);

  auto s3 = reduce(s2, count_down{1});
  CHECK(std::get<A>(s3).value == 5);
  CHECK(std::get<A>(s3).copied == 3);
  CHECK(std::get<B>(s3).copied == 3);

  auto s4 = reduce(s3, multiply{2});
  CHECK(std::get<B>(s4).value == 10);
  CHECK(std::get<A>(s4).copied == 4);
  CHECK(std::get<B>(s4).copied == 4);
}
