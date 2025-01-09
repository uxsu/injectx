#include "injectx/json/parser/stringparser.hpp"

#include <catch2/catch_test_macros.hpp>

namespace injectx::json::tests {

TEST_CASE("parseString") {
  constexpr stdext::static_string str1 = "Hello, World!";
  constexpr std::string_view expected1 = "Hello, World!";
  // Quotes
  constexpr stdext::static_string str2 = R"("Hello, World!")";
  constexpr std::string_view expected2 = "\"Hello, World!\"";
  // All escapable characters except for \u
  constexpr stdext::static_string str3 = R"(\" \/ \\ \b \f \t \n  \r)";
  constexpr std::string_view expected3 = "\" / \\ \b \f \t \n  \r";
  // Hello, World!
  constexpr stdext::static_string str4 =
      R"(\u0048\u0065\u006C\u006C\u006F\u002C\u0020\u0057\u006F\u0072\u006C\u0064\u0021)";
  constexpr std::string_view expected4 = "Hello, World!";
  // すごい
  constexpr stdext::static_string str5 = R"(\u3059\u3054\u3044)";
  constexpr std::string_view expected5 = "すごい";
  // In Japanese, すごい means awesome or something like that :)
  constexpr stdext::static_string str6 =
      R"(In Japanese, \u3059\u3054\u3044 means awesome or something like that :))";
  constexpr std::string_view expected6 =
      "In Japanese, すごい means awesome or something like that :)";

  STATIC_REQUIRE(parser::parseString<str1>() == expected1);
  STATIC_REQUIRE(parser::parseString<str2>() == expected2);
  STATIC_REQUIRE(parser::parseString<str3>() == expected3);
  STATIC_REQUIRE(parser::parseString<str4>() == expected4);
  STATIC_REQUIRE(parser::parseString<str5>() == expected5);
  STATIC_REQUIRE(parser::parseString<str6>() == expected6);
}

}  // namespace injectx::json::tests
