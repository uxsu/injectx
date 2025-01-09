#pragma once

#include <injectx/json/parser/token.hpp>
#include <injectx/json/parser/utility.hpp>
#include <injectx/stdext/expected.hpp>
#include <injectx/stdext/static_string.hpp>

namespace injectx::json::parser {

namespace details {

constexpr char escapedEncoding(char escaped) {
  switch (escaped) {
    case '\\':
      return '\\';
    case '/':
      return '/';
    case 'b':
      return '\b';
    case 'f':
      return '\f';
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case 't':
      return '\t';
    case '"':
      return '"';
  }
  return '\0';
}

constexpr std::uint16_t hexadecimalValue(char c) {
  if (c >= '0' && c <= '9') {
    return std::uint16_t(c - '0');
  } else if (c >= 'a' && c <= 'f') {
    return std::uint16_t(c - 'a' + 10);
  } else {
    return std::uint16_t(c - 'A' + 10);
  }
}

constexpr std::array<char, 3> makeUtf8(std::uint16_t value) {
  if (value < 0x80) {
    std::array<char, 3> buffer = {char(value), '\0', '\0'};
    return buffer;
  } else if (value < 0x800) {
    std::array<char, 3> buffer = {
        char(0xC0 | (value >> 6)), char(0x80 | (value & 0x3F)), '\0'};
    return buffer;
  } else {
    std::array<char, 3> buffer = {
        char(0xE0 | (value >> 12)), char(0x80 | ((value >> 6) & 0x3F)),
        char(0x80 | (value & 0x3F))};
    return buffer;
  }
}

constexpr auto decodeUnicode(char u1, char u2, char u3, char u4) {
  std::uint16_t hex_value =
      hexadecimalValue(u1) * std::uint16_t(4096) +  // 1 << 12
      hexadecimalValue(u2) * std::uint16_t(256) +   // 1 << 8
      hexadecimalValue(u3) * std::uint16_t(16) +    // 1 << 4
      hexadecimalValue(u4);
  return makeUtf8(hex_value);
}

// Assumes that the string uses escape characters within the JSON specification
// rules properly
template<stdext::static_string string>
constexpr auto parseString() {
  constexpr auto calculateParsedSize = []() {
    auto from_iter = string.data_.begin();
    auto end = string.data_.end();
    std::size_t size = 0;
    while (from_iter != end) {
      auto iter = std::find_if(from_iter, end, utility::isBackslash);
      size += static_cast<std::size_t>(std::distance(from_iter, iter));
      if (iter == end) {
        break;
      }
      // Skip backslash
      ++iter;
      if (utility::isUnicodeIndicator(*iter)) {
        // Skip indicator
        ++iter;
        char u1 = *(iter++);
        char u2 = *(iter++);
        char u3 = *(iter++);
        char u4 = *(iter++);
        auto decoded = decodeUnicode(u1, u2, u3, u4);
        size += static_cast<std::size_t>(std::distance(
            decoded.begin(),
            std::find_if(decoded.begin(), decoded.end(), utility::isNull)));
      } else {
        ++iter;
        ++size;
      }
      stdext::expects(iter != from_iter, "Programming error, infinite loop!");
      from_iter = iter;
    }
    return size;
  };
  constexpr auto size = calculateParsedSize();
  stdext::static_string<size> result("");
  auto result_iter = result.data_.begin();
  auto from_iter = string.data_.begin();
  auto end = string.data_.end();
  while (from_iter != end) {
    auto iter = std::find_if(from_iter, end, utility::isBackslash);
    result_iter = std::copy(from_iter, iter, result_iter);
    if (iter == end) {
      break;
    }
    // Skip backslash
    ++iter;
    if (utility::isUnicodeIndicator(*iter)) {
      // Skip indicator
      ++iter;
      char u1 = *(iter++);
      char u2 = *(iter++);
      char u3 = *(iter++);
      char u4 = *(iter++);
      auto decoded = decodeUnicode(u1, u2, u3, u4);
      result_iter = std::copy_if(
          decoded.begin(), decoded.end(), result_iter, !utility::isNull);
    } else {
      *(result_iter++) = escapedEncoding(*(iter++));
    }
    from_iter = iter;
  }
  return result;
};

template<stdext::static_string string>
inline constexpr auto ParsedString = parseString<string>();
}  // namespace details

template<stdext::static_string string>
constexpr std::string_view parseString() {
  return std::string_view(details::ParsedString<string>.data());
}

}  // namespace injectx::json::parser