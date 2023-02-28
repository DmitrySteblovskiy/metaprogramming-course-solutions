#pragma once

#include <optional>

template <class From, auto target>
struct Mapping {
  static constexpr auto Target{target};
};

template <class Base, class Target, class... Mappings>
requires (std::same_as<std::remove_cv_t<typename Mappings::Target>, Target> && ...)
struct PolymorphicMapper {
  static std::optional<Target> map(const Base&) {
    return std::nullopt;
  }
};

template<class Base, class Target, Target tg, class CastTo, class... Mappings> 
struct PolymorphicMapper<Base, Target, Mapping<CastTo, tg>, Mappings...> {
  static std::optional<Target> map(const Base& object) {
  }
};
