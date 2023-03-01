#pragma once

namespace value_types {

template <class T>
concept VTag = requires { T::Value; };

template <auto V>
struct ValueTag {
  static constexpr auto Value = V;
};

template <class T, T... ts>
using VTuple = type_tuples::TTuple<ValueTag<ts>...>;
} // namespace value_types
