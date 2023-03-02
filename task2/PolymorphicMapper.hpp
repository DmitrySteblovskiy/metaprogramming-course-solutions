#pragma once

#include <concepts>
#include <memory>
#include <optional>
#include <type_traits>

template <class From, auto target> struct Mapping {
  using Target = decltype(target);
};

template <class Base, class Target, class... Mappings>
  requires(std::same_as<std::remove_cv_t<typename Mappings::Target>, Target> &&
           ...)
struct PolymorphicMapper {
  static std::optional<Target> map(const Base &) { return std::nullopt; }
};

template <class Base, class Target, Target targ, class Caster,
          class... Mappings>
  requires std::is_base_of_v<Base, Caster>
struct PolymorphicMapper<Base, Target, Mapping<Caster, targ>, Mappings...> {
  static std::optional<Target> map(const Base &object) {
    auto objCastResult = dynamic_cast<const Caster *>(&object);
    if (objCastResult != nullptr) {
      return std::make_optional(targ);
    }
    return PolymorphicMapper<Base, Target, Mappings...>::map(object);
  }
};
