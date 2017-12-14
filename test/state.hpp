#pragma once

#include <tuple>

struct A {
  int value = 0;
  int copied = 0;
  constexpr A(int v, int c = 0) : value{v}, copied{c} {}
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
  int value = 7;
  int copied = 0;
  constexpr B(int v, int c = 0) : value{v}, copied{c} {}
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

constexpr auto initial_state = [] { return std::make_tuple<A, B>(5, 7); };
