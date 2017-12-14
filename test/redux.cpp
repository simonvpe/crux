#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "action.hpp"
#include "reduce.hpp"
#include "state.hpp"

#include <doctest.h>
#include <iostream>
//#include <middleware/crash_reporter.hpp>
#include <middleware/thunk.hpp>
#include <redux.hpp>

SCENARIO("Store") {

  GIVEN("A redux store with two substates") {
    auto store = redux::store{reduce, initial_state()};
    REQUIRE(std::get<A>(store.state()).value == 5);
    REQUIRE(std::get<B>(store.state()).value == 7);

    WHEN("Dispatching a count_up action of 5") {
      store.dispatch(count_up{5});

      THEN("The value of A should be increased by that amount") {
        CHECK(std::get<A>(store.state()).value == 10);
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
        CHECK(std::get<A>(store.state()).value == 3);
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

SCENARIO("Middleware") {

  GIVEN("A redux store with two substates and a single middleware") {
    auto store = redux::store{
        reduce, initial_state(),
        redux::make_middleware(
            [](const auto &, auto &&dispatch, auto &&next, const auto &action) {
              using TAction = std::decay_t<decltype(action)>;
              if constexpr (std::is_same_v<TAction, multiply>) {
                dispatch(count_up{5});
              }
              return next(action);
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

  GIVEN("A redux store with two substates and a two middlewares") {
    auto store = redux::store{
        reduce, std::make_tuple<A, B>(5, 7),
        redux::make_middleware(
            [](const auto &, auto &&dispatch, auto &&next, const auto &action) {
              using TAction = std::decay_t<decltype(action)>;
              if constexpr (std::is_same_v<TAction, multiply>) {
                dispatch(count_up{5});
              }
              return next(action);
            }),
        redux::make_middleware(
            [](const auto &, auto &&dispatch, auto &&next, const auto &action) {
              using TAction = std::decay_t<decltype(action)>;
              if constexpr (std::is_same_v<TAction, count_up>) {
                dispatch(count_down{1});
              }
              return next(action);
            })};

    WHEN("Dispatching a multiply action") {
      store.dispatch(multiply{10});

      THEN("The middlewares should do their thing") {
        CHECK(std::get<A>(store.state()).value == 9);
        CHECK(std::get<B>(store.state()).value == 70);
      }

      AND_THEN("There should be no excessive copying") {
        CHECK(std::get<A>(store.state()).copied == 3);
      }
    }
  }
}

SCENARIO("Thunk Middleware") {

  GIVEN("A redux store with one substate and a thunk middleware") {
    using namespace std::chrono_literals;
    auto store =
        redux::store{reduce, std::make_tuple<A, B>(5, 7), redux::thunk};

    WHEN("Dispatching a callable") {
      auto call_count = 0;
      store.dispatch([&](auto &&dispatch, const auto &state) { call_count++; });
      THEN("The call count should increase") { CHECK(call_count == 1); }
    }

    WHEN("Dispatching an single async action") {
      store
          .dispatch([](auto &&dispatch, const auto &state) {
            return std::async(std::launch::async, [=] {
              std::this_thread::sleep_for(10ms);
              dispatch(count_up{5});
            });
          })
          .wait();

      THEN("It should have fired an action") {
        CHECK(std::get<A>(store.state()).value == 10);
      }
    }
  }
}

/*
SCENARIO("Crash Reporter Middleware") {
  GIVEN("A redux store with two substates and a crash reporter middleware") {
    auto store = redux::store{reduce, std::make_tuple<A, B>(5, 7)};
    WHEN("Dispatching a divide by zero action") {
      THEN("It should not throw") { CHECK_NOTHROW(store.dispatch(divide{0})); }
    }
  }
}
*/
