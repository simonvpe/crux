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

  GIVEN("A redux store with two substates") {
    auto store = redux::store{reduce, A{0}, B{7}};
    REQUIRE(std::get<A>(store.state()).value == 0);

    WHEN("Dispatching a count_up action of 5") {
      store.dispatch(count_up{5});
      THEN("The value of A should be increased by that amount") {
        CHECK(std::get<A>(store.state()).value == 5);
      }
      AND_THEN(
          "The number of times the state have been copied should increase by "
          "one") {
        CHECK(std::get<A>(store.state()).copied == 1);
      }
    }

    WHEN("Dispatching a count_down action of 2") {
      store.dispatch(count_down{2});
      THEN("The value of A should be decreased by that amount") {
        CHECK(std::get<A>(store.state()).value == -2);
      }
      AND_THEN(
          "The number of times the state have been copied should increase by "
          "one") {
        CHECK(std::get<A>(store.state()).copied == 1);
      }
    }

    WHEN("Dispatching a multiply action of 10") {
      store.dispatch(multiply{10});
      THEN("The value of B should be multiplied by that amount") {
        CHECK(std::get<B>(store.state()).value == 70);
      }
      AND_THEN(
          "The number of times the state have been copied should increase by "
          "one") {
        CHECK(std::get<B>(store.state()).copied == 1);
      }
    }
  }
}
