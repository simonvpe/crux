#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <redux.hpp>

#include <variant>
#include <vector>
#include <functional>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

struct up_action {
  const int value;
};

struct down_action {
  const int value;
};

struct mystate {
  int counter = 0;
};

using action_t = std::variant<up_action,down_action>;

template<typename TState, typename TAction, typename...Fs>
constexpr auto reducer(Fs&&...f) {
  return [f...](TState& state, TAction&& action) {
    const auto next = std::visit(overloaded<Fs...>{
      f...
    }, std::variant<TState>{state}, action);
    return next;
  };
};

SCENARIO("Reducer") {
  mystate st;
  const auto combined_reducer = reducer<mystate, action_t>(
    [](mystate state, up_action up) {
      state.counter += up.value;
      return state;
    },
    [](mystate state, down_action down) {
      state.counter -= down.value;
      return state;
    }
  );

  auto state = std::vector<mystate>(1);
  {
    const auto next = combined_reducer(state.back(), {up_action{1}});
    state.push_back( next );
    CHECK( next.counter == 1 );
  }
  {
    const auto next = combined_reducer(state.back(), {down_action{2}});
    state.push_back( next );
    CHECK( next.counter == -1 );
  }
  {
    const auto next = combined_reducer(state.back(), {down_action{0}});
    state.push_back( next );
    CHECK( next.counter == -1 );
  }   
}
