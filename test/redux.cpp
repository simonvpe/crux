#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <redux.hpp>

#include <variant>
#include <vector>
#include <functional>
#include <tuple>

struct up_action {
  const int value = 0;
};

struct down_action {
  const int value = 0;
};

struct counter_state {
  int counter = 0;
};

using counter_action = std::variant<up_action,down_action>;

struct todo {
  std::string text = "";
  bool done = 0.0f;
};

struct new_todo_action {
  std::string text = "";
};

struct todo_state {
  std::vector<todo> todos;
};

using todo_action = std::variant<new_todo_action>;

namespace redux::detail {
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
}  

namespace redux {

template<typename TState, typename TAction, typename F>
struct reducer {
  constexpr reducer(F&& f)
  : f{std::forward<F>(f)}
  {}

  auto operator()(const TState& state, const TAction& action) const {
    return f(state, action);
  }
  
  const F f;
};
  
template<typename TState, typename TAction, typename...Fs>
constexpr auto make_reducer(Fs&&...f) {
  auto reduce = [f...](const TState& state, const TAction& action) {
    const auto next = std::visit(detail::overloaded<Fs...>{
      f...
    }, std::variant<TState>{state}, action);
    return next;
  };
  return reducer<TState, TAction, decltype(reduce)>{std::move(reduce)};
};

template<typename...Fs>  
constexpr auto combine_reducers(Fs...f) {
  return std::make_tuple(std::variant<Fs...>{f}...);
}

}

SCENARIO("Combine reducers") {
  using namespace redux;
  
  const auto counter_reducer = make_reducer<counter_state, counter_action>(
    [](counter_state state, up_action up) {
      state.counter += up.value;
      return state;
    },
    [](counter_state state, down_action down) {
      state.counter -= down.value;
      return state;
    }
  );

  const auto todo_reducer = make_reducer<todo_state, todo_action>(
    [](todo_state state, new_todo_action n) {
      state.todos.push_back({ .text = n.text, .done = false });
      return state;
    }
  );

  combine_reducers(counter_reducer, todo_reducer);
  
  //  const auto store = std::make_tuple(counter_state{}, todo_state{})
  //    | counter_reducer
  //    | todo_reducer
  //    ;
}

SCENARIO("Reducer") {
  counter_state st;
  const auto reducer = redux::make_reducer<counter_state, counter_action>(
    [](counter_state state, up_action up) {
      state.counter += up.value;
      return state;
    },
    [](counter_state state, down_action down) {
      state.counter -= down.value;
      return state;
    }
  );

  auto state = std::vector<counter_state>(1);
  {
    const auto next = reducer(state.back(), {up_action{1}});
    state.push_back( next );
    CHECK( next.counter == 1 );
  }
  {
    const auto next = reducer(state.back(), {down_action{2}});
    state.push_back( next );
    CHECK( next.counter == -1 );
  }
  {
    const auto next = reducer(state.back(), {down_action{0}});
    state.push_back( next );
    CHECK( next.counter == -1 );
  }   
}
