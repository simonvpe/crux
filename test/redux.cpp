#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <redux.hpp>

#include <array>
#include <functional>
#include <tuple>
#include <variant>
#include <vector>

struct A {
  int value = 0;
  int copied = 0;
  A(int v) : value{v} {}
  A(A &&other)
      : value{std::move(other.value)}, copied{std::move(other.copied)} {}
  A(const A &other) {
    value = other.value;
    copied = other.copied + 1;
  }
  A &operator=(const A &other) {
    value = other.value;
    copied = other.copied + 1;
    return *this;
  }
};

struct B {
  int value = 0;
  int copied = 0;
  B(int v) : value{v} {}
  B(B &&other)
      : value{std::move(other.value)}, copied{std::move(other.copied)} {}
  B(const B &other) {
    value = other.value;
    copied = other.copied + 1;
  }
  B &operator=(const B &other) {
    value = other.value;
    copied = other.copied + 1;
    return *this;
  }
};

namespace redux::detail {
template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;
} // namespace redux::detail

namespace redux {
auto combine_reducers(auto &&... fs) {
  const auto dispatch = detail::overloaded{fs...};
  return [&](const auto &state, const auto &action) {
    const auto f = [&](const auto &s) {
      return std::visit(
          [&](const auto &a) -> std::remove_reference_t<decltype(s)> {
            return std::move(dispatch(s, a));
          },
          action);
    };
    const auto g = [&](const auto &... s) { return std::make_tuple(f(s)...); };
    return std::apply(g, state);
  };
}
} // namespace redux

SCENARIO("Reduce") {
  struct count_up {
    const int value = 0;
  };

  struct count_down {
    const int value = 0;
  };

  struct multiply {
    const int value = 0;
  };

  auto reduce = redux::combine_reducers(
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
      },
      [](const auto &state, const auto &) { return state; });

  using V = std::variant<count_up, count_down, multiply>;
  auto s0 = std::make_tuple<A, B>(A{0}, B{5});
  CHECK(std::get<A>(s0).copied == 0);
  CHECK(std::get<B>(s0).copied == 0);

  auto s1 = reduce(s0, V{count_up{3}});
  CHECK(std::get<A>(s1).value == 3);
  CHECK(std::get<A>(s1).copied == 1);
  CHECK(std::get<B>(s1).copied == 1);

  auto s2 = reduce(s1, V{count_up{3}});
  CHECK(std::get<A>(s2).value == 6);
  CHECK(std::get<A>(s2).copied == 2);
  CHECK(std::get<B>(s2).copied == 2);

  auto s3 = reduce(s2, V{count_down{1}});
  CHECK(std::get<A>(s3).value == 5);
  CHECK(std::get<A>(s3).copied == 3);
  CHECK(std::get<B>(s3).copied == 3);

  auto s4 = reduce(s3, V{multiply{2}});
  CHECK(std::get<B>(s4).value == 10);
  CHECK(std::get<A>(s4).copied == 4);
  CHECK(std::get<B>(s4).copied == 4);
}
