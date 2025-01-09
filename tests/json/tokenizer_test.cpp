#include "injectx/json/parser/tokenizer.hpp"

#include <catch2/catch_test_macros.hpp>

namespace injectx::json::tests {

TEST_CASE("tokenizer") {
  struct TokenCounter {
    std::size_t count_ = 0;

    stdext::expected<void, std::string_view> operator()(parser::Token) {
      ++count_;
      return {};
    }
  };
  TokenCounter counter;
  SECTION("Empty object") {
    parser::details::JsonReader json("{}");
    auto result = parser::details::consumeDocument(json, counter);
    REQUIRE(result.has_value());
    REQUIRE(counter.count_ == 2);
  }
  SECTION("Empty array") {
    parser::details::JsonReader json("{}");
    auto result = parser::details::consumeDocument(json, counter);
    REQUIRE(result.has_value());
    REQUIRE(counter.count_ == 2);
  }
}


}  // namespace injectx::json::tests