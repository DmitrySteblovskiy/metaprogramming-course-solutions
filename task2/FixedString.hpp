#pragma once

#include <string>
#include <string_view>

template <size_t max_length> struct FixedString {
  size_t len;
  const char res_str[256];

  constexpr FixedString(const char *string, size_t length) : len(length) {
    for (size_t i = 0; i < len; ++i)
      res_str[i] = string[i];
  }

  constexpr operator std::string_view() const {
    return std::string_view(res_str, len);
  }
};

constexpr FixedString<256> operator"" _cstr(const char *str, size_t len) {
  return FixedString<256>(str, len);
}