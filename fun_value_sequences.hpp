#pragma once

#include <value_types.hpp>

using type_lists::Iterate;
using Nats = Iterate<PlusPlus, value_types::ValueTag<0>>;

template <value_types::VTag T> struct CheckSimpleness {
  static constexpr bool Value = 
};

using type_lists::Filter;
using Primes = Filter<CheckSimpleness, Nats>;

template <class T1, class T2> struct AntFib {
  using Head = Sum<T1, T2>;
  using Tail = AntFib<T2, Head>;
};

using Fib =