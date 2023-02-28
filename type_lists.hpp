#pragma once

#include <concepts>
#include <type_tuples.hpp>

namespace type_lists {

template <class TL>
concept TypeSequence = requires {
                         typename TL::Head;
                         typename TL::Tail;
                       };

struct Nil {};

template <class TL>
concept Empty = std::derived_from<TL, Nil>;

template <class TL>
concept TypeList = Empty<TL> || TypeSequence<TL>;

template <class T, TypeList TL> struct Cons {
  using Head = T;
  using Tail = TL;
};

template <class T> struct Repeat {
  using Head = T;
  using Tail = Repeat<T>;
};

} // namespace type_lists
