#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <redux.hpp>

#include <array>
#include <functional>
#include <tuple>
#include <variant>
#include <vector>

struct A {
  constexpr A(int v = 0) : value{v} {}
  int value;
};
struct B {
  constexpr B(int v = 0) : value{v} {}
  int value;
};

namespace redux::detail {
template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;

template <typename... Ts>
constexpr auto unpack(const std::tuple<Ts...> &value) {
  return std::apply(
      [](auto &... x) { return std::array{std::variant<Ts...>{x}...}; }, value);
}

template <typename... Ts, std::size_t N>
constexpr auto pack(const std::array<std::variant<Ts...>, N> &arr) {
  return std::apply(
      [](auto &... x) { return std::tuple<Ts...>{std::get<Ts>(x)...}; }, arr);
}
} // namespace redux::detail

namespace redux {
auto combine_reducers(auto &&... fs) {
  return [fs...](auto state, auto action) {
    auto unpacked = detail::unpack(state);
    for (auto &substate : unpacked) {
      std::visit(detail::overloaded{fs...}, substate, action);
    }
    // return std::make_tuple<A, B>(A{3}, B{4});
    return detail::pack(unpacked);
  };
}
} // namespace redux

SCENARIO("Combine Reducers") {
  struct count_up {
    constexpr count_up(int v) : value{v} {}
    int value;
  };

  struct count_down {
    constexpr count_down(int v) : value{v} {}
    int value;
  };

  struct multiply {
    constexpr multiply(int v) : value{v} {}
    int value;
  };

  constexpr auto cstate = std::make_tuple<A, B>(A{0}, B{5});

  auto reduce = redux::combine_reducers(
      [](A &state, const count_up &action) { state.value += action.value; },
      [](A &state, const count_down &action) { state.value -= action.value; },
      [](B &state, const multiply &action) { state.value *= action.value; },
      [](auto &, const auto &) {});

  using V = std::variant<count_up, count_down, multiply>;

  auto s1 = reduce(cstate, V{count_up{3}});
  CHECK(std::get<A>(s1).value == 3);

  auto s2 = reduce(s1, V{count_up{3}});
  CHECK(std::get<A>(s2).value == 6);

  auto s3 = reduce(s2, V{count_down{1}});
  CHECK(std::get<A>(s3).value == 5);

  auto s4 = reduce(s3, V{multiply{2}});
  CHECK(std::get<B>(s4).value == 10);
}

SCENARIO("Pack") {

  using V = std::variant<A, B>;
  constexpr auto src = std::array{V{A{1}}, V{B{2}}};
  constexpr auto dst = redux::detail::pack(src);

  CHECK(std::get<0>(dst).value == 1);
  CHECK(std::get<1>(dst).value == 2);
}

SCENARIO("Unpack") {
  constexpr auto src = std::make_tuple<A, B>(A{1}, B{2});
  constexpr auto dst = redux::detail::unpack(src);

  CHECK(std::get<A>(dst[0]).value == 1);
  CHECK(std::get<B>(dst[1]).value == 2);
}
