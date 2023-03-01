#pragma once

#include <cmath>
#include <concepts>
#include <type_tuples.hpp>

namespace type_lists { // nm grouper...

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

template <template <class> class F, TypeList TL> struct Map {
  using Head = F<typename TL::Head>;
  using Tail = Map<F, typename TL::Tail>;
};

template <template <class> class F, Empty TL> struct Map<F, TL> : public Nil {};

template <class TT> struct CycleDef;

template <class T, class... Ts> using type_tuples::TTuple;
struct CycleDef<TTuple<T, Ts...>> { // t_t
  using Head = T;
  using type_tuples::TTuple;
  using Tail = CycleDef<TTuple<Ts..., T>>;
};

template <TypeList TL> using Cycle = CycleDef<ToTuple<TL>>; // to

namespace makers {
template <class TL, class TT> struct ToTupleMake;

template <TypeList TL, class... Ts>
struct ToTupleMake<TL, type_tuples::TTuple<Ts...>> {
  using Type =
      typename ToTupleMake<typename TL::Tail,
                           type_tuples::TTuple<Ts..., typename TL::Head>>::Type;
};

template <Empty TL, class... Ts>
struct ToTupleMake<TL, type_tuples::TTuple<Ts...>> {
  using Type = type_tuples::TTuple<Ts...>;
};

template <template <class, class> class OP, class T, TypeList TL>
struct ScanlMaker
    : public Cons<OP<T, typename TL::Head>,
                  ScanlMaker<OP, OP<T, typename TL::Head>, typename TL::Tail>> {
};

template <template <class, class> class OP, class T, Empty TL>
struct ScanlMaker<OP, T, TL> : public Nil {};

template <template <class, class> class OP, class T, TypeList TL>
struct FoldlMaker {
  using Type = typename FoldlMaker<OP, OP<T, typename TL::Head>,
                                   typename TL::Tail>::Type;
};

template <template <class, class> class OP, class T, Empty TL>
struct FoldlMaker<OP, T, TL> {
  using Type = T;
};

} // namespace makers

template <TypeList TL>
using ToTuple = typename makers::ToTupleMake<TL, type_tuples::TTuple<>>::Type;

template <template <class> class P, TypeList TL> struct Filter {
  using Head = typename TL::Head;
  using Tail = Filter<P, typename TL::Tail>;
};

template <template <class> class P, TypeList TL>
  requires(!P<typename TL::Head>::Value)
struct Filter<P, TL> : Filter<P, typename TL::Tail> {};

template <template <class> class P, Empty TL> struct Filter<P, TL> : Nil {};

template <template <class, class> class OP, class T, TypeList TL>
using Scanl = Cons<T, makers::ScanlMaker<OP, T, TL>>;

template <template <class, class> class OP, class T, TypeList TL>
using Foldl = typename makers::FoldlMaker<OP, T, TL>::Type;

template <TypeList TL>
struct Tails : public Cons<TL, Tails<typename TL::Tail>> {};

template <Empty TL> struct Tails<TL> : public Cons<Nil, Nil> {};

template <class... Ts> struct FromTuple : public Nil {};

template <class T, class... Ts>
struct FromTuple<type_tuples::TTuple<T, Ts...>>
    : public Cons<T, FromTuple<type_tuples::TTuple<Ts...>>> {};

template <TypeList TL, class... Init>
struct Inits
    : public Cons<FromTuple<type_tuples::TTuple<Init...>>,
                  Inits<typename TL::Tail, Init..., typename TL::Head>> {};

template <Empty TL, class... Init>
struct Inits<TL, Init...>
    : public Cons<FromTuple<type_tuples::TTuple<Init...>>, Nil> {};

template <TypeList First, TypeList Second> struct Zip2 {
  using Head = type_tuples::TTuple<typename First::Head, typename Second::Head>;

  using Tail = Zip2<typename First::Tail, typename Second::Tail>;
};

template <Empty First, TypeList Second> struct Zip2<First, Second> : Nil {};

template <TypeList First, Empty Second> struct Zip2<First, Second> : Nil {};

template <TypeList... TL> struct Zip {
  using Head = type_tuples::TTuple<typename TL::Head...>;
  using Tail = Zip<typename TL::Tail...>;
};

namespace grouper {
template <template <class, class> class EQ, TypeList TL, class First>
constexpr int GroupSize() {
  if constexpr (std::is_base_of_v<Nil, TL>) {
    return 0;
  } else if constexpr (std::is_base_of_v<Nil, typename TL::Head>) {
    return 0;
  } else if constexpr (EQ<First, typename TL::Head>::Value) {
    using TL::Tail;
    int curr_res = GroupSize<EQ, Tail, First>() + 1; // tl typename
    return curr_res;
  } else {
    return 0;
  }
}
} // namespace grouper

template <template <typename, typename> typename EQ, TypeList TL>
struct GroupBy {
  using Head = Take<grouper::GroupSize<EQ, TL, typename TL::Head>(), TL>;
  using Tail =
      GroupBy<EQ, Drop<grouper::GroupSize<EQ, TL, typename TL::Head>(), TL>>;
};

template <template <typename, typename> typename EQ, Empty TL>
struct GroupBy<EQ, TL> : Nil {};
} // namespace type_lists
