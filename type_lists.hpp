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

template <int N, TypeList TL>
struct Take : public Cons<typename TL::Head, Take<N - 1, typename TL::Tail>> {};

template <int N, Empty TL> struct Take<N, TL> : public Nil {};

template <TypeList TL> struct Take<0, TL> : public Nil {};

template <int N, TypeList TL>
struct Drop : public Drop<N - 1, typename TL::Tail> {};

template <TypeList TL> struct Drop<0, TL> : public TL {};

template <int N, Empty TL> struct Drop<N, TL> : public Nil {};

template <int N, class T> using Replicate = Take<N, Repeat<T>>;

template <template <class> class F, class T> struct Iterate {
  using Head = T;
  using Tail = Iterate<F, F<T>>;
};

template <class TL, class TT> struct ToTupleImpl;

template <TypeList TL, class... Ts>
struct ToTupleImpl<TL, type_tuples::TTuple<Ts...>> {
  using Type =
      typename ToTupleImpl<typename TL::Tail,
                           type_tuples::TTuple<Ts..., typename TL::Head>>::Type;
};

template <Empty TL, class... Ts>
struct ToTupleImpl<TL, type_tuples::TTuple<Ts...>> {
  using Type = type_tuples::TTuple<Ts...>;
};

template <TypeList TL>
using ToTuple = typename ToTupleImpl<TL, type_tuples::TTuple<>>::Type;

template <class... Ts> struct FromTuple : public Nil {};

template <class T, class... Ts>
struct FromTuple<type_tuples::TTuple<T, Ts...>>
    : public Cons<T, FromTuple<type_tuples::TTuple<Ts...>>> {};
} // namespace type_lists
