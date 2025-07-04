#include "injectx/json/parser/tokenizer.hpp"

#include <catch2/catch_test_macros.hpp>

#include <vector>

namespace injectx::json::tests {
struct TokenExtractor {
  std::vector<parser::Token> tokens_{};

  constexpr stdext::expected<void, parser::ParseError> operator()(
      parser::Token token) {
    tokens_.push_back(token);

    return {};
  }
};

using ConsumeResultExpactation =
    std::vector<std::pair<parser::Token::Type, std::string_view>>;

inline constexpr stdext::expected<void, std::string_view> requiresPass(
    parser::ConsumerType consumerType,
    std::string_view json,
    const ConsumeResultExpactation& expectation) {
  TokenExtractor extractor;
  auto extractor_result = parser::selectConsumeFunction<TokenExtractor>(
      consumerType)({.json = json}, extractor);
  if (!extractor_result.has_value()) {
    return stdext::unexpected{extractor_result.error().message_};
  }
  auto size = extractor.tokens_.size();
  if (size != expectation.size()) {
    return stdext::unexpected{"Size mismatch!"};
  }
  for (std::size_t i = 0; i < size; ++i) {
    if (extractor.tokens_[i].type_ != expectation[i].first) {
      return stdext::unexpected{"Type mismatch!"};
    }
    if (extractor.tokens_[i].string_ != expectation[i].second) {
      return stdext::unexpected{"String mismatch!"};
    }
  }

  return {};
}

inline constexpr stdext::expected<void, std::string_view> requiresFail(
    parser::ConsumerType consumerType,
    std::string_view json,
    std::string_view expectedError) {
  TokenExtractor extractor;
  auto extractor_result = parser::selectConsumeFunction<TokenExtractor>(
      consumerType)({.json = json}, extractor);
  if (extractor_result.has_value()) {
    return stdext::unexpected{"Unexpectedly extracted tokens!"};
  }
  if (extractor_result.error().message_ != expectedError) {
    return stdext::unexpected{"Error mismatch"};
  }
  return {};
}

inline constexpr auto requiresPassNull(std::string_view json) {
  return requiresPass(
      parser::ConsumerType::Null, json, {{parser::Token::Type::Null, "null"}});
}

inline constexpr auto requiresFailNull(std::string_view json) {
  return requiresFail(
      parser::ConsumerType::Null, json, "Expected null but couldn't parse it");
}

inline constexpr auto requiresPassBoolean(
    std::string_view json, bool expectation) {
  using namespace std::string_view_literals;
  return requiresPass(
      parser::ConsumerType::Boolean, json,
      {{parser::Token::Type::Boolean, expectation ? "true"sv : "false"sv}});
}

inline constexpr auto requiresFailBoolean(std::string_view json) {
  return requiresFail(
      parser::ConsumerType::Boolean, json,
      "Expected a boolean but couldn't parse it");
}

inline constexpr auto requiresPassNumber(
    std::string_view json, std::string_view expectation) {
  using namespace std::string_view_literals;
  return requiresPass(
      parser::ConsumerType::Number, json,
      {{parser::Token::Type::Number, expectation}});
}

inline constexpr auto requiresFailNumber(
    std::string_view json, std::string_view errorExpactation) {
  return requiresFail(parser::ConsumerType::Number, json, errorExpactation);
}

inline constexpr auto requiresPassString(
    std::string_view json, std::string_view expectation) {
  using namespace std::string_view_literals;
  return requiresPass(
      parser::ConsumerType::String, json,
      {{parser::Token::Type::String, expectation}});
}

inline constexpr auto requiresFailString(
    std::string_view json, std::string_view errorExpactation) {
  return requiresFail(parser::ConsumerType::String, json, errorExpactation);
}

inline constexpr auto requiresPassValue(
    std::string_view json,
    parser::Token::Type type,
    std::string_view expectation) {
  using namespace std::string_view_literals;
  return requiresPass(parser::ConsumerType::Value, json, {{type, expectation}});
}

inline constexpr auto requiresFailValue(
    std::string_view json, std::string_view errorExpactation) {
  return requiresFail(parser::ConsumerType::Value, json, errorExpactation);
}

inline constexpr auto requiresPassArray(
    std::string_view json, const ConsumeResultExpactation& expectation) {
  using namespace std::string_view_literals;
  return requiresPass(parser::ConsumerType::Array, json, expectation);
}

inline constexpr auto requiresFailArray(
    std::string_view json, std::string_view errorExpactation) {
  return requiresFail(parser::ConsumerType::Array, json, errorExpactation);
}

inline constexpr auto requiresPassObject(
    std::string_view json, const ConsumeResultExpactation& expectation) {
  using namespace std::string_view_literals;
  return requiresPass(parser::ConsumerType::Object, json, expectation);
}

inline constexpr auto requiresFailObject(
    std::string_view json, std::string_view errorExpactation) {
  return requiresFail(parser::ConsumerType::Object, json, errorExpactation);
}

inline constexpr auto requiresPassDocument(
    std::string_view json, const ConsumeResultExpactation& expectation) {
  using namespace std::string_view_literals;
  return requiresPass(parser::ConsumerType::Document, json, expectation);
}

inline constexpr auto requiresFailDocument(
    std::string_view json, std::string_view errorExpactation) {
  return requiresFail(parser::ConsumerType::Document, json, errorExpactation);
}

TEST_CASE("consumeNull") {
  SECTION("positive") {
    STATIC_REQUIRE(requiresPassNull("null").has_value());
    STATIC_REQUIRE(requiresPassNull("null  ").has_value());
    STATIC_REQUIRE(requiresPassNull("nullkdlsakl ").has_value());
    STATIC_REQUIRE(requiresPassNull("nullnull").has_value());
    STATIC_REQUIRE(requiresPassNull("null,").has_value());
  }
  SECTION("negative") {
    STATIC_REQUIRE(requiresFailNull("nul l").has_value());
    STATIC_REQUIRE(requiresFailNull("nul").has_value());
    STATIC_REQUIRE(requiresFailNull(" nullkdlsakl ").has_value());
    STATIC_REQUIRE(requiresFailNull("nnullnull").has_value());
    STATIC_REQUIRE(requiresFailNull(",null,").has_value());
  }
}

TEST_CASE("consumeBoolean") {
  SECTION("positive-exact-true") {
    STATIC_REQUIRE(requiresPassBoolean("true", true).has_value());
    STATIC_REQUIRE(requiresPassBoolean("true  ", true).has_value());
    STATIC_REQUIRE(requiresPassBoolean("truekdlsakl ", true).has_value());
    STATIC_REQUIRE(requiresPassBoolean("truetrue", true).has_value());
    STATIC_REQUIRE(requiresPassBoolean("true,", true).has_value());
    STATIC_REQUIRE(requiresPassBoolean("false", false).has_value());
    STATIC_REQUIRE(requiresPassBoolean("false  ", false).has_value());
    STATIC_REQUIRE(requiresPassBoolean("falsekdlsakl ", false).has_value());
    STATIC_REQUIRE(requiresPassBoolean("falsefalse", false).has_value());
    STATIC_REQUIRE(requiresPassBoolean("false,", false).has_value());
  }
  SECTION("negative") {
    STATIC_REQUIRE(requiresFailBoolean(" true").has_value());
    STATIC_REQUIRE(requiresFailBoolean("ttrue  ").has_value());
    STATIC_REQUIRE(requiresFailBoolean("ftruekdlsakl ").has_value());
    STATIC_REQUIRE(requiresFailBoolean("trutrue").has_value());
    STATIC_REQUIRE(requiresFailBoolean("truue,").has_value());
    STATIC_REQUIRE(requiresFailBoolean(" false").has_value());
    STATIC_REQUIRE(requiresFailBoolean("ffalse  ").has_value());
    STATIC_REQUIRE(requiresFailBoolean("tfalsekdlsakl ").has_value());
    STATIC_REQUIRE(requiresFailBoolean("falsfalse").has_value());
    STATIC_REQUIRE(requiresFailBoolean("falsse,").has_value());
  }
}

TEST_CASE("consumeNumber") {
  SECTION("positive-integer") {
    STATIC_REQUIRE(requiresPassNumber("8934523", "8934523").has_value());
    STATIC_REQUIRE(requiresPassNumber("-9053290 ", "-9053290").has_value());
    STATIC_REQUIRE(requiresPassNumber("0\n", "0").has_value());
    STATIC_REQUIRE(requiresPassNumber("-0%", "-0").has_value());
    STATIC_REQUIRE(requiresPassNumber(
                       "8934523995834958493589435849582930432589230859320890,",
                       "8934523995834958493589435849582930432589230859320890")
                       .has_value());
    STATIC_REQUIRE(requiresPassNumber(
                       "-8934523995834958493589435849582930432589230859320890]",
                       "-8934523995834958493589435849582930432589230859320890")
                       .has_value());
  }
  SECTION("positive-integer-with-fraction") {
    STATIC_REQUIRE(
        requiresPassNumber("654.932049320592095302}", "654.932049320592095302")
            .has_value());
    STATIC_REQUIRE(requiresPassNumber(
                       "-905329532.395034859328{", "-905329532.395034859328")
                       .has_value());
    STATIC_REQUIRE(
        requiresPassNumber("0.9538958293$", "0.9538958293").has_value());
    STATIC_REQUIRE(requiresPassNumber("-0.8594358698239\t", "-0.8594358698239")
                       .has_value());
    STATIC_REQUIRE(
        requiresPassNumber(
            "90543905960950350259023592309.905238592385293502359802\b",
            "90543905960950350259023592309.905238592385293502359802")
            .has_value());
    STATIC_REQUIRE(
        requiresPassNumber(
            "-90543905960950350259023592309.905238592385293502359802",
            "-90543905960950350259023592309.905238592385293502359802")
            .has_value());
  }
  SECTION("positive-integer-with-exponent") {
    STATIC_REQUIRE(requiresPassNumber("654e32", "654e32").has_value());
    STATIC_REQUIRE(requiresPassNumber("-654e32", "-654e32").has_value());
    STATIC_REQUIRE(requiresPassNumber("654e-32", "654e-32").has_value());
    STATIC_REQUIRE(requiresPassNumber("-654e-32", "-654e-32").has_value());
    STATIC_REQUIRE(requiresPassNumber("654e+32", "654e+32").has_value());
    STATIC_REQUIRE(requiresPassNumber("-654e+32", "-654e+32").has_value());
    STATIC_REQUIRE(requiresPassNumber("-654e+0", "-654e+0").has_value());
    STATIC_REQUIRE(requiresPassNumber("-654e-0", "-654e-0").has_value());
    STATIC_REQUIRE(requiresPassNumber("-0e-0", "-0e-0").has_value());
    STATIC_REQUIRE(requiresPassNumber("-0e+0", "-0e+0").has_value());
    STATIC_REQUIRE(requiresPassNumber("0e-0", "0e-0").has_value());
    STATIC_REQUIRE(requiresPassNumber("0e+0", "0e+0").has_value());
    STATIC_REQUIRE(requiresPassNumber("654E32", "654E32").has_value());
    STATIC_REQUIRE(requiresPassNumber("-654E32", "-654E32").has_value());
    STATIC_REQUIRE(requiresPassNumber("654E-32", "654E-32").has_value());
    STATIC_REQUIRE(requiresPassNumber("-654E-32", "-654E-32").has_value());
    STATIC_REQUIRE(requiresPassNumber("654E+32", "654E+32").has_value());
    STATIC_REQUIRE(requiresPassNumber("-654E+32", "-654E+32").has_value());
    STATIC_REQUIRE(requiresPassNumber("-654E+0", "-654E+0").has_value());
    STATIC_REQUIRE(requiresPassNumber("-654E-0", "-654E-0").has_value());
    STATIC_REQUIRE(requiresPassNumber("-0E-0", "-0E-0").has_value());
    STATIC_REQUIRE(requiresPassNumber("-0E+0", "-0E+0").has_value());
    STATIC_REQUIRE(requiresPassNumber("0E-0", "0E-0").has_value());
    STATIC_REQUIRE(requiresPassNumber("0E+0", "0E+0").has_value());
  }
  SECTION("positive-integer-with-fraction-and-exponent") {
    STATIC_REQUIRE(
        requiresPassNumber("48394328.5049053e332", "48394328.5049053e332")
            .has_value());
    STATIC_REQUIRE(
        requiresPassNumber("-48394328.5049053e32", "-48394328.5049053e32")
            .has_value());
    STATIC_REQUIRE(
        requiresPassNumber("48394328.5049053e+38", "48394328.5049053e+38")
            .has_value());
    STATIC_REQUIRE(
        requiresPassNumber("48394328.5049053e-38", "48394328.5049053e-38")
            .has_value());
  }
  SECTION("negative") {
    STATIC_REQUIRE(requiresFailNumber("+954839593", "Expected a digit or '-'")
                       .has_value());
    STATIC_REQUIRE(
        requiresFailNumber("-.94032", "'-' should be followed by a digit")
            .has_value());
    STATIC_REQUIRE(
        requiresFailNumber("-0.e123", "'.' should be followed by a digit")
            .has_value());
    STATIC_REQUIRE(
        requiresFailNumber(
            "-0.1Ea123", "'e(+/-)' or 'E(+/-)' should be followed by a digit")
            .has_value());
    STATIC_REQUIRE(
        requiresFailNumber(
            "-0.1e+a123", "'e(+/-)' or 'E(+/-)' should be followed by a digit")
            .has_value());
    STATIC_REQUIRE(
        requiresFailNumber(
            "-0.1E-a123", "'e(+/-)' or 'E(+/-)' should be followed by a digit")
            .has_value());
  }
}

TEST_CASE("consumeString") {
  SECTION("positive") {
    STATIC_REQUIRE(requiresPassString(R"("")", R"()").has_value());
    STATIC_REQUIRE(
        requiresPassString(R"("8934523")", R"(8934523)").has_value());

    STATIC_REQUIRE(
        requiresPassString(R"("\"8934523")", R"(\"8934523)").has_value());
    STATIC_REQUIRE(
        requiresPassString(
            R"("\"89   lfewkl 34523" we are not interested in what happens after ending quote)",
            R"(\"89   lfewkl 34523)")
            .has_value());
    STATIC_REQUIRE(requiresPassString(
                       R"("\\ \/ \b \f \r \t \n \u432AD" kbrejgiw)",
                       R"(\\ \/ \b \f \r \t \n \u432AD)")
                       .has_value());
  }

  SECTION("negative") {
    STATIC_REQUIRE(
        requiresFailString(R"( 8934523)", "Strings must begin with '\"'")
            .has_value());
    STATIC_REQUIRE(requiresFailString(
                       R"("8934523)",
                       "Reached to end while looking for an enclosing '\"' "
                       "for the string")
                       .has_value());
    STATIC_REQUIRE(requiresFailString(
                       R"("8934523\")",
                       "Reached to end while looking for an enclosing '\"' "
                       "for the string")
                       .has_value());
    STATIC_REQUIRE(requiresFailString(
                       R"("\x")",
                       "Backslashes should be followed by one of the"
                       "escapable characters: \",b,f,n,r,t,u,\\,/")
                       .has_value());
    STATIC_REQUIRE(requiresFailString(
                       R"("\v")",
                       "Backslashes should be followed by one of the"
                       "escapable characters: \",b,f,n,r,t,u,\\,/")
                       .has_value());
    STATIC_REQUIRE(requiresFailString(
                       R"("\u")",
                       "\\u should be followed by 4 hexadecimal digits "
                       "for proper unicode escaping")
                       .has_value());
    STATIC_REQUIRE(requiresFailString(
                       R"("\u123")",
                       "\\u should be followed by 4 hexadecimal digits "
                       "for proper unicode escaping")
                       .has_value());
    STATIC_REQUIRE(requiresFailString(
                       R"("\u231G")",
                       "\\u should be followed by 4 hexadecimal digits "
                       "for proper unicode escaping")
                       .has_value());
    STATIC_REQUIRE(requiresFailString(
                       R"("\u231x")",
                       "\\u should be followed by 4 hexadecimal digits "
                       "for proper unicode escaping")
                       .has_value());
  }
}

TEST_CASE("consumeValue") {
  SECTION("positive") {
    STATIC_REQUIRE(requiresPassValue("null", parser::Token::Type::Null, "null")
                       .has_value());
    STATIC_REQUIRE(
        requiresPassValue("null  ", parser::Token::Type::Null, "null")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("nullkdlsakl ", parser::Token::Type::Null, "null")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("nullnull", parser::Token::Type::Null, "null")
            .has_value());
    STATIC_REQUIRE(requiresPassValue("null,", parser::Token::Type::Null, "null")
                       .has_value());
    STATIC_REQUIRE(
        requiresPassValue("true", parser::Token::Type::Boolean, "true")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("true  ", parser::Token::Type::Boolean, "true")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("truekdlsakl ", parser::Token::Type::Boolean, "true")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("truetrue", parser::Token::Type::Boolean, "true")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("true,", parser::Token::Type::Boolean, "true")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("false", parser::Token::Type::Boolean, "false")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("false  ", parser::Token::Type::Boolean, "false")
            .has_value());
    STATIC_REQUIRE(requiresPassValue(
                       "falsekdlsakl ", parser::Token::Type::Boolean, "false")
                       .has_value());
    STATIC_REQUIRE(
        requiresPassValue("falsefalse", parser::Token::Type::Boolean, "false")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("false,", parser::Token::Type::Boolean, "false")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("8934523", parser::Token::Type::Number, "8934523")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("-9053290 ", parser::Token::Type::Number, "-9053290")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("0\n", parser::Token::Type::Number, "0").has_value());
    STATIC_REQUIRE(requiresPassValue("-0%", parser::Token::Type::Number, "-0")
                       .has_value());
    STATIC_REQUIRE(requiresPassValue(
                       "8934523995834958493589435849582930432589230859320890,",
                       parser::Token::Type::Number,
                       "8934523995834958493589435849582930432589230859320890")
                       .has_value());
    STATIC_REQUIRE(requiresPassValue(
                       "-8934523995834958493589435849582930432589230859320890]",
                       parser::Token::Type::Number,
                       "-8934523995834958493589435849582930432589230859320890")
                       .has_value());
    STATIC_REQUIRE(requiresPassValue(
                       "654.932049320592095302}", parser::Token::Type::Number,
                       "654.932049320592095302")
                       .has_value());
    STATIC_REQUIRE(requiresPassValue(
                       "-905329532.395034859328{", parser::Token::Type::Number,
                       "-905329532.395034859328")
                       .has_value());
    STATIC_REQUIRE(
        requiresPassValue(
            "0.9538958293$", parser::Token::Type::Number, "0.9538958293")
            .has_value());
    STATIC_REQUIRE(requiresPassValue(
                       "-0.8594358698239\t", parser::Token::Type::Number,
                       "-0.8594358698239")
                       .has_value());
    STATIC_REQUIRE(
        requiresPassValue(
            "90543905960950350259023592309.905238592385293502359802\b",
            parser::Token::Type::Number,
            "90543905960950350259023592309.905238592385293502359802")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue(
            "-90543905960950350259023592309.905238592385293502359802",
            parser::Token::Type::Number,
            "-90543905960950350259023592309.905238592385293502359802")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("654e32", parser::Token::Type::Number, "654e32")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("-654e32", parser::Token::Type::Number, "-654e32")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("654e-32", parser::Token::Type::Number, "654e-32")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("-654e-32", parser::Token::Type::Number, "-654e-32")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("654e+32", parser::Token::Type::Number, "654e+32")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("-654e+32", parser::Token::Type::Number, "-654e+32")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("-654e+0", parser::Token::Type::Number, "-654e+0")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("-654e-0", parser::Token::Type::Number, "-654e-0")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("-0e-0", parser::Token::Type::Number, "-0e-0")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("-0e+0", parser::Token::Type::Number, "-0e+0")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("0e-0", parser::Token::Type::Number, "0e-0")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("0e+0", parser::Token::Type::Number, "0e+0")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("654E32", parser::Token::Type::Number, "654E32")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("-654E32", parser::Token::Type::Number, "-654E32")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("654E-32", parser::Token::Type::Number, "654E-32")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("-654E-32", parser::Token::Type::Number, "-654E-32")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("654E+32", parser::Token::Type::Number, "654E+32")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("-654E+32", parser::Token::Type::Number, "-654E+32")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("-654E+0", parser::Token::Type::Number, "-654E+0")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("-654E-0", parser::Token::Type::Number, "-654E-0")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("-0E-0", parser::Token::Type::Number, "-0E-0")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("-0E+0", parser::Token::Type::Number, "-0E+0")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("0E-0", parser::Token::Type::Number, "0E-0")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue("0E+0", parser::Token::Type::Number, "0E+0")
            .has_value());
    STATIC_REQUIRE(requiresPassValue(
                       "48394328.5049053e332", parser::Token::Type::Number,
                       "48394328.5049053e332")
                       .has_value());
    STATIC_REQUIRE(requiresPassValue(
                       "-48394328.5049053e32", parser::Token::Type::Number,
                       "-48394328.5049053e32")
                       .has_value());
    STATIC_REQUIRE(requiresPassValue(
                       "48394328.5049053e+38", parser::Token::Type::Number,
                       "48394328.5049053e+38")
                       .has_value());
    STATIC_REQUIRE(requiresPassValue(
                       "48394328.5049053e-38", parser::Token::Type::Number,
                       "48394328.5049053e-38")
                       .has_value());
    STATIC_REQUIRE(
        requiresPassValue(R"("")", parser::Token::Type::String, R"()")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue(
            R"("8934523")", parser::Token::Type::String, R"(8934523)")
            .has_value());

    STATIC_REQUIRE(
        requiresPassValue(
            R"("\"8934523")", parser::Token::Type::String, R"(\"8934523)")
            .has_value());
    STATIC_REQUIRE(
        requiresPassValue(
            R"("\"89   lfewkl 34523" we are not interested in what happens
        after
            ending quote)",
            parser::Token::Type::String, R"(\"89   lfewkl 34523)")
            .has_value());
    STATIC_REQUIRE(requiresPassValue(
                       R"("\\ \/ \b \f \r \t \n \u432AD" kbrejgiw)",
                       parser::Token::Type::String,
                       R"(\\ \/ \b \f \r \t \n \u432AD)")
                       .has_value());
  }
  SECTION("negative") {
    STATIC_REQUIRE(
        requiresFailValue("nul l", "Expected null but couldn't parse it")
            .has_value());
    STATIC_REQUIRE(
        requiresFailValue("nul", "Expected null but couldn't parse it")
            .has_value());
    STATIC_REQUIRE(requiresFailValue(
                       " nullkdlsakl ",
                       "Values can either be an object, an array, a string, a "
                       "number, true, false or null")
                       .has_value());
    STATIC_REQUIRE(
        requiresFailValue("nnullnull", "Expected null but couldn't parse it")
            .has_value());
    STATIC_REQUIRE(requiresFailValue(
                       ",null,",
                       "Values can either be an object, an array, a string, a "
                       "number, true, false or null")
                       .has_value());
    STATIC_REQUIRE(requiresFailValue(
                       " true",
                       "Values can either be an object, an array, a string, a "
                       "number, true, false or null")
                       .has_value());
    STATIC_REQUIRE(
        requiresFailValue("ttrue  ", "Expected a boolean but couldn't parse it")
            .has_value());
    STATIC_REQUIRE(
        requiresFailValue(
            "ftruekdlsakl ", "Expected a boolean but couldn't parse it")
            .has_value());
    STATIC_REQUIRE(
        requiresFailValue("trutrue", "Expected a boolean but couldn't parse it")
            .has_value());
    STATIC_REQUIRE(
        requiresFailValue("truue,", "Expected a boolean but couldn't parse it")
            .has_value());
    STATIC_REQUIRE(requiresFailValue(
                       " false",
                       "Values can either be an object, an array, a string, a "
                       "number, true, false or null")
                       .has_value());
    STATIC_REQUIRE(requiresFailValue(
                       "ffalse  ", "Expected a boolean but couldn't parse it")
                       .has_value());
    STATIC_REQUIRE(
        requiresFailValue(
            "tfalsekdlsakl ", "Expected a boolean but couldn't parse it")
            .has_value());
    STATIC_REQUIRE(requiresFailValue(
                       "falsfalse", "Expected a boolean but couldn't parse it")
                       .has_value());
    STATIC_REQUIRE(
        requiresFailValue("falsse,", "Expected a boolean but couldn't parse it")
            .has_value());
    STATIC_REQUIRE(requiresFailValue(
                       "+954839593",
                       "Values can either be an object, an array, a string, a "
                       "number, true, false or null")
                       .has_value());
    STATIC_REQUIRE(
        requiresFailValue("-.94032", "'-' should be followed by a digit")
            .has_value());
    STATIC_REQUIRE(
        requiresFailValue("-0.e123", "'.' should be followed by a digit")
            .has_value());
    STATIC_REQUIRE(
        requiresFailValue(
            "-0.1Ea123", "'e(+/-)' or 'E(+/-)' should be followed by a digit")
            .has_value());
    STATIC_REQUIRE(
        requiresFailValue(
            "-0.1e+a123", "'e(+/-)' or 'E(+/-)' should be followed by a digit")
            .has_value());
    STATIC_REQUIRE(
        requiresFailValue(
            "-0.1E-a123", "'e(+/-)' or 'E(+/-)' should be followed by a digit")
            .has_value());
    STATIC_REQUIRE(requiresFailValue(
                       R"( 8934523)",
                       "Values can either be an object, an array, a string, a "
                       "number, true, false or null")
                       .has_value());
    STATIC_REQUIRE(requiresFailValue(
                       R"("8934523)",
                       "Reached to end while looking for an enclosing '\"' "
                       "for the string")
                       .has_value());
    STATIC_REQUIRE(requiresFailValue(
                       R"("8934523\")",
                       "Reached to end while looking for an enclosing '\"' "
                       "for the string")
                       .has_value());
    STATIC_REQUIRE(requiresFailValue(
                       R"("\x")",
                       "Backslashes should be followed by one of the"
                       "escapable characters: \",b,f,n,r,t,u,\\,/")
                       .has_value());
    STATIC_REQUIRE(requiresFailValue(
                       R"("\v")",
                       "Backslashes should be followed by one of the"
                       "escapable characters: \",b,f,n,r,t,u,\\,/")
                       .has_value());
    STATIC_REQUIRE(requiresFailValue(
                       R"("\u")",
                       "\\u should be followed by 4 hexadecimal digits "
                       "for proper unicode escaping")
                       .has_value());
    STATIC_REQUIRE(requiresFailValue(
                       R"("\u123")",
                       "\\u should be followed by 4 hexadecimal digits "
                       "for proper unicode escaping")
                       .has_value());
    STATIC_REQUIRE(requiresFailValue(
                       R"("\u231G")",
                       "\\u should be followed by 4 hexadecimal digits "
                       "for proper unicode escaping")
                       .has_value());
    STATIC_REQUIRE(requiresFailValue(
                       R"("\u231x")",
                       "\\u should be followed by 4 hexadecimal digits "
                       "for proper unicode escaping")
                       .has_value());
  }
}

TEST_CASE("consumeArray") {
  SECTION("positive") {
    STATIC_REQUIRE(requiresPassArray(
                       R"([
      "string", 123e45, true, false, null, [], {}
])",
                       {
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::String, "string"},
                           {parser::Token::Type::Number, "123e45"},
                           {parser::Token::Type::Boolean, "true"},
                           {parser::Token::Type::Boolean, "false"},
                           {parser::Token::Type::Null, "null"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::ObjectEnd, "}"},
                           {parser::Token::Type::ArrayEnd, "]"},
                       })
                       .has_value());
    STATIC_REQUIRE(requiresPassArray(
                       "[]",
                       {
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayEnd, "]"},
                       })
                       .has_value());

    STATIC_REQUIRE(requiresPassArray(
                       "[1, 2, 3]",
                       {
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::Number, "1"},
                           {parser::Token::Type::Number, "2"},
                           {parser::Token::Type::Number, "3"},
                           {parser::Token::Type::ArrayEnd, "]"},
                       })
                       .has_value());

    STATIC_REQUIRE(requiresPassArray(
                       "[true, false, null]",
                       {
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::Boolean, "true"},
                           {parser::Token::Type::Boolean, "false"},
                           {parser::Token::Type::Null, "null"},
                           {parser::Token::Type::ArrayEnd, "]"},
                       })
                       .has_value());

    STATIC_REQUIRE(requiresPassArray(
                       R"([["nested", [123], []], "end"])",
                       {
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::String, "nested"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::Number, "123"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::String, "end"},
                           {parser::Token::Type::ArrayEnd, "]"},
                       })
                       .has_value());
    STATIC_REQUIRE(requiresPassArray(
                       "[[], [[]], [[], []]]",
                       {
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ArrayEnd, "]"},
                       })
                       .has_value());

    STATIC_REQUIRE(requiresPassArray(
                       R"(["text", 123, {}, [true, null], false])",
                       {
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::String, "text"},
                           {parser::Token::Type::Number, "123"},
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::ObjectEnd, "}"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::Boolean, "true"},
                           {parser::Token::Type::Null, "null"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::Boolean, "false"},
                           {parser::Token::Type::ArrayEnd, "]"},
                       })
                       .has_value());
  }
  SECTION("negative") {
    STATIC_REQUIRE(
        requiresFailArray("[", "Arrays must end with ']'").has_value());

    STATIC_REQUIRE(
        requiresFailArray("]", "Arrays must begin with '['").has_value());
    STATIC_REQUIRE(
        requiresFailArray("[[1]", "Reached to end while looking for ',' or ']'")
            .has_value());
    STATIC_REQUIRE(
        requiresFailArray("[1 2]", "Expected ']' or ','").has_value());
    STATIC_REQUIRE(
        requiresFailArray("[[1],", "Reached to end while looking for a value")
            .has_value());
    STATIC_REQUIRE(
        requiresFailArray("[1, 2, ]", "',' should not be followed by ']'")
            .has_value());

    STATIC_REQUIRE(
        requiresFailArray("[null, true false]", "Expected ']' or ','")
            .has_value());
  }
}

TEST_CASE("consumeObject") {
  SECTION("positive") {
    STATIC_REQUIRE(requiresPassObject(
                       "{}",
                       {
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::ObjectEnd, "}"},
                       })
                       .has_value());

    STATIC_REQUIRE(requiresPassObject(
                       R"({"key": "value"})",
                       {
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "key"},
                           {parser::Token::Type::String, "value"},
                           {parser::Token::Type::ObjectEnd, "}"},
                       })
                       .has_value());

    STATIC_REQUIRE(requiresPassObject(
                       R"({"a":1, "b":true, "c":null})",
                       {
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "a"},
                           {parser::Token::Type::Number, "1"},
                           {parser::Token::Type::String, "b"},
                           {parser::Token::Type::Boolean, "true"},
                           {parser::Token::Type::String, "c"},
                           {parser::Token::Type::Null, "null"},
                           {parser::Token::Type::ObjectEnd, "}"},
                       })
                       .has_value());

    STATIC_REQUIRE(requiresPassObject(
                       R"({"nested": {"inner": 42}})",
                       {
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "nested"},
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "inner"},
                           {parser::Token::Type::Number, "42"},
                           {parser::Token::Type::ObjectEnd, "}"},
                           {parser::Token::Type::ObjectEnd, "}"},
                       })
                       .has_value());

    STATIC_REQUIRE(requiresPassObject(
                       R"({"arr": [1, 2, 3]})",
                       {
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "arr"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::Number, "1"},
                           {parser::Token::Type::Number, "2"},
                           {parser::Token::Type::Number, "3"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ObjectEnd, "}"},
                       })
                       .has_value());
    STATIC_REQUIRE(requiresPassObject(
                       R"({
                            "name": "Example",
                            "attributes": {
                                "id": 123,
                                "active": true,
                                "tags": ["alpha", "beta", "gamma", "delta"]
                            },
                            "metrics": {
                                "scores": [1, 2, 3, 4, 5, 6],
                                "flags": [true, false, null]
                            },
                            "data": {
                                "items": [
                                    {"key": "a", "value": 1},
                                    {"key": "b", "value": 2}
                                ]
                            }
                        })",
                       {

                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "name"},
                           {parser::Token::Type::String, "Example"},
                           {parser::Token::Type::String, "attributes"},
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "id"},
                           {parser::Token::Type::Number, "123"},
                           {parser::Token::Type::String, "active"},
                           {parser::Token::Type::Boolean, "true"},
                           {parser::Token::Type::String, "tags"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::String, "alpha"},
                           {parser::Token::Type::String, "beta"},
                           {parser::Token::Type::String, "gamma"},
                           {parser::Token::Type::String, "delta"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ObjectEnd, "}"},
                           {parser::Token::Type::String, "metrics"},
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "scores"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::Number, "1"},
                           {parser::Token::Type::Number, "2"},
                           {parser::Token::Type::Number, "3"},
                           {parser::Token::Type::Number, "4"},
                           {parser::Token::Type::Number, "5"},
                           {parser::Token::Type::Number, "6"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::String, "flags"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::Boolean, "true"},
                           {parser::Token::Type::Boolean, "false"},
                           {parser::Token::Type::Null, "null"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ObjectEnd, "}"},
                           {parser::Token::Type::String, "data"},
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "items"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "key"},
                           {parser::Token::Type::String, "a"},
                           {parser::Token::Type::String, "value"},
                           {parser::Token::Type::Number, "1"},
                           {parser::Token::Type::ObjectEnd, "}"},
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "key"},
                           {parser::Token::Type::String, "b"},
                           {parser::Token::Type::String, "value"},
                           {parser::Token::Type::Number, "2"},
                           {parser::Token::Type::ObjectEnd, "}"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ObjectEnd, "}"},
                           {parser::Token::Type::ObjectEnd, "}"},
                       })
                       .has_value());
  }
  SECTION("negative") {
    STATIC_REQUIRE(requiresFailObject(
                       R"("arr": [1, 2, 3]})", "Objects must begin with '{'")
                       .has_value());
    STATIC_REQUIRE(
        requiresFailObject(R"({)", "Objects must end with '}'").has_value());
    STATIC_REQUIRE(
        requiresFailObject(R"({"arr")", "Reached to end while looking for ':'")
            .has_value());
    STATIC_REQUIRE(requiresFailObject(
                       R"({"arr",)", "key-value pairs must be separated by ','")
                       .has_value());
    STATIC_REQUIRE(requiresFailObject(
                       R"({"arr":)", "Reached to end while looking for a value")
                       .has_value());
    STATIC_REQUIRE(
        requiresFailObject(
            R"({"arr":"value")", "Reached to end while looking for ',' or '}'")
            .has_value());
    STATIC_REQUIRE(
        requiresFailObject(
            R"({"arr":"value",)", "Reached to end while looking for a key")
            .has_value());
    STATIC_REQUIRE(
        requiresFailObject(
            R"({"arr":"value",})", "',' should not be followed by '}'")
            .has_value());
  }
}

TEST_CASE("consumeDocument") {
  SECTION("positive") {
    STATIC_REQUIRE(requiresPassDocument(
                       R"(
[
      "string", 123e45, true, false, null, [], {}
])",
                       {
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::String, "string"},
                           {parser::Token::Type::Number, "123e45"},
                           {parser::Token::Type::Boolean, "true"},
                           {parser::Token::Type::Boolean, "false"},
                           {parser::Token::Type::Null, "null"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::ObjectEnd, "}"},
                           {parser::Token::Type::ArrayEnd, "]"},
                       })
                       .has_value());
    STATIC_REQUIRE(requiresPassDocument(
                       "[]",
                       {
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayEnd, "]"},
                       })
                       .has_value());

    STATIC_REQUIRE(requiresPassDocument(
                       "[1, 2, 3]",
                       {
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::Number, "1"},
                           {parser::Token::Type::Number, "2"},
                           {parser::Token::Type::Number, "3"},
                           {parser::Token::Type::ArrayEnd, "]"},
                       })
                       .has_value());

    STATIC_REQUIRE(requiresPassDocument(
                       "   [true, false, null]",
                       {
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::Boolean, "true"},
                           {parser::Token::Type::Boolean, "false"},
                           {parser::Token::Type::Null, "null"},
                           {parser::Token::Type::ArrayEnd, "]"},
                       })
                       .has_value());

    STATIC_REQUIRE(requiresPassDocument(
                       R"(            [["nested", [123], []], "end"]        )",
                       {
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::String, "nested"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::Number, "123"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::String, "end"},
                           {parser::Token::Type::ArrayEnd, "]"},
                       })
                       .has_value());
    STATIC_REQUIRE(requiresPassDocument(
                       "[[], [[]], [[], []]]            ",
                       {
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ArrayEnd, "]"},
                       })
                       .has_value());

    STATIC_REQUIRE(requiresPassDocument(
                       R"(["text", 123, {}, [true, null], false] 
                         


                         )",
                       {
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::String, "text"},
                           {parser::Token::Type::Number, "123"},
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::ObjectEnd, "}"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::Boolean, "true"},
                           {parser::Token::Type::Null, "null"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::Boolean, "false"},
                           {parser::Token::Type::ArrayEnd, "]"},
                       })
                       .has_value());
    STATIC_REQUIRE(requiresPassDocument(
                       "{}",
                       {
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::ObjectEnd, "}"},
                       })
                       .has_value());

    STATIC_REQUIRE(requiresPassDocument(
                       R"({"key": "value"})",
                       {
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "key"},
                           {parser::Token::Type::String, "value"},
                           {parser::Token::Type::ObjectEnd, "}"},
                       })
                       .has_value());

    STATIC_REQUIRE(requiresPassDocument(
                       R"({"a":1, "b":true, "c":null})",
                       {
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "a"},
                           {parser::Token::Type::Number, "1"},
                           {parser::Token::Type::String, "b"},
                           {parser::Token::Type::Boolean, "true"},
                           {parser::Token::Type::String, "c"},
                           {parser::Token::Type::Null, "null"},
                           {parser::Token::Type::ObjectEnd, "}"},
                       })
                       .has_value());

    STATIC_REQUIRE(requiresPassDocument(
                       R"({"nested": {"inner": 42}})",
                       {
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "nested"},
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "inner"},
                           {parser::Token::Type::Number, "42"},
                           {parser::Token::Type::ObjectEnd, "}"},
                           {parser::Token::Type::ObjectEnd, "}"},
                       })
                       .has_value());

    STATIC_REQUIRE(requiresPassDocument(
                       R"({"arr": [1, 2, 3]})",
                       {
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "arr"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::Number, "1"},
                           {parser::Token::Type::Number, "2"},
                           {parser::Token::Type::Number, "3"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ObjectEnd, "}"},
                       })
                       .has_value());
    STATIC_REQUIRE(requiresPassDocument(
                       R"(
                         
         
                         
           
                         
                         
                         
           
                         
                         
                         
                         
                         
                         
                         
                         
                         
                         
                         {
                             "name": "Example",
                             "attributes": {
                                 "id": 123,
                                 "active": true,
                                 "tags": ["alpha", "beta", "gamma", "delta"]
                             },
                             "metrics": {
                                 "scores": [1, 2, 3, 4, 5, 6],
                                 "flags": [true, false, null]
                             },
                             "data": {
                                 "items": [
                                     {"key": "a", "value": 1},
                                     {"key": "b", "value": 2}
                                 ]
                             }
                         }
                             
                         
                         
                         
        )",
                       {
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "name"},
                           {parser::Token::Type::String, "Example"},
                           {parser::Token::Type::String, "attributes"},
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "id"},
                           {parser::Token::Type::Number, "123"},
                           {parser::Token::Type::String, "active"},
                           {parser::Token::Type::Boolean, "true"},
                           {parser::Token::Type::String, "tags"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::String, "alpha"},
                           {parser::Token::Type::String, "beta"},
                           {parser::Token::Type::String, "gamma"},
                           {parser::Token::Type::String, "delta"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ObjectEnd, "}"},
                           {parser::Token::Type::String, "metrics"},
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "scores"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::Number, "1"},
                           {parser::Token::Type::Number, "2"},
                           {parser::Token::Type::Number, "3"},
                           {parser::Token::Type::Number, "4"},
                           {parser::Token::Type::Number, "5"},
                           {parser::Token::Type::Number, "6"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::String, "flags"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::Boolean, "true"},
                           {parser::Token::Type::Boolean, "false"},
                           {parser::Token::Type::Null, "null"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ObjectEnd, "}"},
                           {parser::Token::Type::String, "data"},
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "items"},
                           {parser::Token::Type::ArrayBegin, "["},
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "key"},
                           {parser::Token::Type::String, "a"},
                           {parser::Token::Type::String, "value"},
                           {parser::Token::Type::Number, "1"},
                           {parser::Token::Type::ObjectEnd, "}"},
                           {parser::Token::Type::ObjectBegin, "{"},
                           {parser::Token::Type::String, "key"},
                           {parser::Token::Type::String, "b"},
                           {parser::Token::Type::String, "value"},
                           {parser::Token::Type::Number, "2"},
                           {parser::Token::Type::ObjectEnd, "}"},
                           {parser::Token::Type::ArrayEnd, "]"},
                           {parser::Token::Type::ObjectEnd, "}"},
                           {parser::Token::Type::ObjectEnd, "}"},
                       })
                       .has_value());
  }
  SECTION("negative") {
    STATIC_REQUIRE(requiresFailDocument(
                       R"(
        
)",
                       "Empty document")
                       .has_value());
    STATIC_REQUIRE(
        requiresFailDocument("[", "Arrays must end with ']'").has_value());

    STATIC_REQUIRE(requiresFailDocument(
                       "]", "Document must start with an object or an array")
                       .has_value());
    STATIC_REQUIRE(requiresFailDocument(
                       "[[1]", "Reached to end while looking for ',' or ']'")
                       .has_value());
    STATIC_REQUIRE(
        requiresFailDocument("[1 2]", "Expected ']' or ','").has_value());
    STATIC_REQUIRE(requiresFailDocument(
                       "[[1],", "Reached to end while looking for a value")
                       .has_value());
    STATIC_REQUIRE(
        requiresFailDocument("[1, 2, ]", "',' should not be followed by ']'")
            .has_value());

    STATIC_REQUIRE(
        requiresFailDocument("[null, true false]", "Expected ']' or ','")
            .has_value());
    STATIC_REQUIRE(requiresFailDocument(
                       R"("arr": [1, 2, 3]})",
                       "Document must start with an object or an array")
                       .has_value());
    STATIC_REQUIRE(
        requiresFailDocument(R"({)", "Objects must end with '}'").has_value());
    STATIC_REQUIRE(requiresFailDocument(
                       R"({"arr")", "Reached to end while looking for ':'")
                       .has_value());
    STATIC_REQUIRE(requiresFailDocument(
                       R"({"arr",)", "key-value pairs must be separated by ','")
                       .has_value());
    STATIC_REQUIRE(requiresFailDocument(
                       R"({"arr":)", "Reached to end while looking for a value")
                       .has_value());
    STATIC_REQUIRE(
        requiresFailDocument(
            R"({"arr":"value")", "Reached to end while looking for ',' or '}'")
            .has_value());
    STATIC_REQUIRE(
        requiresFailDocument(
            R"({"arr":"value",)", "Reached to end while looking for a key")
            .has_value());
    STATIC_REQUIRE(
        requiresFailDocument(
            R"({"arr":"value",})", "',' should not be followed by '}'")
            .has_value());

    STATIC_REQUIRE(requiresFailDocument(
                       R"({"arr":"value"} {})",
                       "Parsing document is finished yet there are still "
                       "non-whitespace characters")
                       .has_value());
  }
}

}  // namespace injectx::json::tests