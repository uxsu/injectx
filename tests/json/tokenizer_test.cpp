#include "injectx/json/parser/tokenizer.hpp"

#include <catch2/catch_test_macros.hpp>

#include <vector>

namespace injectx::json::tests {

TEST_CASE("consumeDocument") {
  struct TokenCounter {
    std::size_t count_ = 0;

    stdext::expected<void, std::string_view> operator()(parser::Token) {
      ++count_;
      return {};
    }
  };
  TokenCounter counter;
  struct TokenExtractor {
    std::vector<parser::Token> tokens_{};

    stdext::expected<void, std::string_view> operator()(parser::Token token) {
      tokens_.push_back(token);
      return {};
    }
  };
  TokenExtractor extractor;
  SECTION("Empty object") {
    parser::details::JsonReader json("{}");
    auto count_result = parser::details::consumeDocument(json, counter);
    auto extractor_result = parser::details::consumeDocument(json, extractor);
    REQUIRE(count_result.has_value());
    REQUIRE(counter.count_ == 2);
    REQUIRE(extractor_result.has_value());
    REQUIRE(extractor.tokens_.size() == 2);
    REQUIRE(extractor.tokens_[0].type_ == parser::Token::Type::ObjectBegin);
    REQUIRE(extractor.tokens_[0].string_ == "{");
    REQUIRE(extractor.tokens_[1].type_ == parser::Token::Type::ObjectEnd);
    REQUIRE(extractor.tokens_[1].string_ == "}");

  }
  SECTION("Empty array") {
    parser::details::JsonReader json("[]");
    auto count_result = parser::details::consumeDocument(json, counter);
    auto extractor_result = parser::details::consumeDocument(json, extractor);
    REQUIRE(count_result.has_value());
    REQUIRE(counter.count_ == 2);
    REQUIRE(extractor_result.has_value());
    REQUIRE(extractor.tokens_.size() == 2);
    REQUIRE(extractor.tokens_[0].type_ == parser::Token::Type::ArrayBegin);
    REQUIRE(extractor.tokens_[0].string_ == "[");
    REQUIRE(extractor.tokens_[1].type_ == parser::Token::Type::ArrayEnd);
    REQUIRE(extractor.tokens_[1].string_ == "]");
  }
}


}  // namespace injectx::json::tests