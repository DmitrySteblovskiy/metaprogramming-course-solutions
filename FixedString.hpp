#pragma once

#include <string>
#include <string_view>

template<size_t max_length>
struct FixedString {
  constexpr FixedString(const char* string, size_t length): len(length) {
    for (size_t i = 0; i < len; ++ i)
      impl[i] = string[i];
  }

  constexpr operator std::string_view() const {
    return std::string_view(impl, len);
  }
  
  size_t len;
  char impl[256];
};

constexpr FixedString<256> operator ""_cstr(const char* str, size_t len) {
  return FixedString<256>(str, len);
}