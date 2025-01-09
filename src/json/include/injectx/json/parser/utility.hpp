#pragma once

#include "injectx/args/args.hpp"

#include <algorithm>
#include <string_view>

namespace injectx::json::parser::utility {

constexpr auto isNull = args::arg == '\0';

constexpr auto isSpace = args::arg == ' ' || args::arg == '\t'
                      || args::arg == '\n' || args::arg == '\r';

constexpr auto isZero = args::arg == '0';
constexpr auto isDigit = args::arg >= '0' && args::arg <= '9';
constexpr auto isMinus = args::arg == '-';
constexpr auto isPlus = args::arg == '+';
constexpr auto canNumberStartWith = isMinus || isDigit;
constexpr auto isDot = args::arg == '.';

constexpr auto isQuote = args::arg == '"';
constexpr auto isLeftBrace = args::arg == '{';
constexpr auto isRightBrace = args::arg == '}';
constexpr auto isLeftBracket = args::arg == '[';
constexpr auto isRightBracket = args::arg == ']';
constexpr auto isColon = args::arg == ':';
constexpr auto isComma = args::arg == '.';
constexpr auto isBackslash = args::arg == '\\';
constexpr auto isForwardslash = args::arg == '/';
constexpr auto isUnicodeIndicator = args::arg == 'u';

constexpr auto canBooleanStartWith = args::arg == 't' || args::arg == 'f';
constexpr auto canNullStartWith = args::arg == 'n';

constexpr auto canEscape = isQuote || isBackslash || isForwardslash
                        || args::arg == 'b' || args::arg == 'f'
                        || args::arg == 'n' || args::arg == 'r'
                        || args::arg == 't' || args::arg == 'u';

constexpr auto isHexadecimal = isDigit || (args::arg >= 'a' && args::arg <= 'f')
                            || (args::arg >= 'A' && args::arg <= 'F');

constexpr auto isExponent = args::arg == 'e' || args::arg == 'E';

constexpr bool hasBackslash(std::string_view str) {
  auto iter = std::find_if(str.cbegin(), str.cend(), isBackslash);
  return iter != str.cend();
}

}  // namespace injectx::json::parser::utility
