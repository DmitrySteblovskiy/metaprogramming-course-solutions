#pragma once

#include <cmath>
#include <value_types.hpp>

template <class T> using NextOne = value_types::ValueTag<T::Value + 1>;
using Nats = type_lists::Iterate<NextOne, value_types::ValueTag<0>>;


constexpr bool CheckPrimeness(int val, int deriv) {
  if (val <= 1) {
    return false;
  }
  if (std::pow(deriv, 2) > val) {
    return true;
  }
  if (!(val % deriv)) {
    return false;
  }

  return CheckPrimeness(val, deriv + 1);
}

template <value_types::VTag T> struct SimpleVal {
  static constexpr bool Value = CheckPrimeness(T::Value, 2);
};
using Primes = type_lists::Filter<SimpleVal, Nats>;


template <class T1, class T2>
using Sum = value_types::ValueTag<T1::Value + T2::Value>;

template <class T1, class T2> struct AntFib {
  using Head = Sum<T1, T2>;
  using Tail = AntFib<T2, Head>;
};

template <class T1, class T2>
using type_lists::Cons;
using NextFibV = Cons<T1, Cons<T2, AntFib<T1, T2>>>;
using value_types::ValueTag;
using Fib = NextFibV<value_types::ValueTag<0>, value_types::ValueTag<1>>;