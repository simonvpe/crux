#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <iostream>
#include <redux.hpp>

struct A {
  int value = 0;
  int copied = 0;
  A(int v) : value{v} {}
  A(A &&) = default;
  A(const A &other) {
    std::cout << "  A(&)\n";
    value = other.value;
    copied = other.copied + 1;
  }
  A &operator=(const A &other) {
    std::cout << "  operator=\n";
    value = other.value;
    copied = other.copied + 1;
    return *this;
  }
  A &operator=(A &&) = default;
};

struct B {
  int value = 7;
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
      std::cout << "reduce count_up{" << action.value << "}\n";
      auto next_state = state;
      next_state.value += action.value;
      return next_state;
    },
    [](const A &state, const count_down &action) {
      std::cout << "reduce count_down{" << action.value << "}\n";
      auto next_state = state;
      next_state.value -= action.value;
      return next_state;
    },
    [](const B &state, const multiply &action) {
      std::cout << "reduce multiply{" << action.value << "}\n";
      auto next_state = state;
      next_state.value *= action.value;
      return next_state;
    });

SCENARIO("Middleware") {
  GIVEN("A redux store with two substates and a single middleware") {

    auto store = redux::store{
        reduce, std::make_tuple<A, B>(5, 7),
        redux::make_middleware(
            [](const auto &, auto &&dispatch, auto &&next, const auto &action) {
              using TAction = std::decay_t<decltype(action)>;
              if constexpr (std::is_same_v<TAction, multiply>) {
                dispatch(count_up{5});
              }
              next(action);
            })};

    WHEN("Dispatching a multiply action") {
      store.dispatch(multiply{5});
      THEN("The value of B should have been multiplied") {
        CHECK(std::get<B>(store.state()).value == 35);
      }
      AND_THEN("The middleware should have been called (and should have "
               "counted up A)") {
        CHECK(std::get<A>(store.state()).value == 10);
      }
      AND_THEN("The state should have been copied twice") {
        CHECK(std::get<A>(store.state()).copied == 2);
      }
    }
  }
}

SCENARIO("Store") {

  GIVEN("A redux store with two substates") {
    auto store = redux::store{reduce, std::make_tuple<A, B>(0, 7)};
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

    WHEN("Registering a subscriber") {
      auto subscribe_triggered = 0;
      const auto unsubscribe =
          store.subscribe([&] { subscribe_triggered += 1; });
      AND_WHEN("Dispatching an action") {
        store.dispatch(count_up{1});
        THEN("The subscriber should be triggered once") {
          CHECK(subscribe_triggered == 1);
        }
      }
      AND_WHEN("Unsubscribing") {
        unsubscribe();
        AND_WHEN("Dispatching an action") {
          store.dispatch(count_up{1});
          THEN("The subscriber should not be triggered") {
            CHECK(subscribe_triggered == 0);
          }
        }
      }
    }
  }
}
