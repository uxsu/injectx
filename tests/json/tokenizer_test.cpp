#include "injectx/json/parser/tokenizer.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>

#define STATIC_REQUIRE_COUNTER_AND_EXTRACTOR(Counter, Extractor, Json, Type) \
  constexpr auto results = getCounterAndExtractor<Type, []() {               \
    return Json;                                                             \
  }>();                                                                      \
  constexpr auto counter_result = results.first;                             \
  constexpr auto extractor_result = results.second;                          \
  STATIC_REQUIRE(counter_result.has_value());                                \
  constexpr auto Counter = counter_result.value();                           \
  (void)Counter;                                                             \
  STATIC_REQUIRE(extractor_result.has_value());                              \
  constexpr auto Extractor = extractor_result.value();                       \
  (void)Extractor;

#define STATIC_REQUIRE_FALSE_COUNTER_AND_EXTRACTOR(            \
    Counter, Extractor, Json, Type)                            \
  constexpr auto results = getCounterAndExtractor<Type, []() { \
    return Json;                                               \
  }>();                                                        \
  constexpr auto counter_result = results.first;               \
  constexpr auto extractor_result = results.second;            \
  STATIC_REQUIRE_FALSE(counter_result.has_value());            \
  constexpr auto Counter = counter_result.error();             \
  (void)Counter;                                               \
  STATIC_REQUIRE_FALSE(extractor_result.has_value());          \
  constexpr auto Extractor = extractor_result.error();         \
  (void)Extractor;

#define STATIC_REQUIRE_NULL_TEST_PASS(NULLTEXT)                              \
  {                                                                          \
    constexpr auto json = NULLTEXT;                                          \
    STATIC_REQUIRE_COUNTER_AND_EXTRACTOR(                                    \
        counter, extractor, json, parser::ConsumerType::Null);               \
    STATIC_REQUIRE(counter.count_ == 1);                                     \
    STATIC_REQUIRE(extractor.index_ == 1);                                   \
    STATIC_REQUIRE(extractor.tokens_[0].type_ == parser::Token::Type::Null); \
    STATIC_REQUIRE(extractor.tokens_[0].string_ == "null");                  \
  }

#define STATIC_REQUIRE_NULL_TEST_FAIL(NULLTEXT, MESSAGE)                   \
  {                                                                        \
    constexpr auto json = NULLTEXT;                                        \
    STATIC_REQUIRE_FALSE_COUNTER_AND_EXTRACTOR(                            \
        counter_error, extractor_error, json, parser::ConsumerType::Null); \
    STATIC_REQUIRE(counter_error.message_ == MESSAGE);                     \
    STATIC_REQUIRE(extractor_error.message_ == MESSAGE);                   \
  }

#define STATIC_REQUIRE_BOOLEAN_TEST_PASS(BOOLEAN, EXPECTED)          \
  {                                                                  \
    constexpr auto json = BOOLEAN;                                   \
    STATIC_REQUIRE_COUNTER_AND_EXTRACTOR(                            \
        counter, extractor, json, parser::ConsumerType::Boolean);    \
    STATIC_REQUIRE(counter.count_ == 1);                             \
    STATIC_REQUIRE(extractor.index_ == 1);                           \
    STATIC_REQUIRE(                                                  \
        extractor.tokens_[0].type_ == parser::Token::Type::Boolean); \
    STATIC_REQUIRE(extractor.tokens_[0].string_ == EXPECTED);        \
  }

#define STATIC_REQUIRE_BOOLEAN_TEST_FAIL(BOOLEAN, MESSAGE)                    \
  {                                                                           \
    constexpr auto json = BOOLEAN;                                            \
    STATIC_REQUIRE_FALSE_COUNTER_AND_EXTRACTOR(                               \
        counter_error, extractor_error, json, parser::ConsumerType::Boolean); \
    STATIC_REQUIRE(counter_error.message_ == MESSAGE);                        \
    STATIC_REQUIRE(extractor_error.message_ == MESSAGE);                      \
  }

#define STATIC_REQUIRE_NUMBER_TEST_PASS(NUMBER, EXPECTED)                      \
  {                                                                            \
    constexpr auto json = NUMBER;                                              \
    STATIC_REQUIRE_COUNTER_AND_EXTRACTOR(                                      \
        counter, extractor, json, parser::ConsumerType::Number);               \
    STATIC_REQUIRE(counter.count_ == 1);                                       \
    STATIC_REQUIRE(extractor.index_ == 1);                                     \
    STATIC_REQUIRE(extractor.tokens_[0].type_ == parser::Token::Type::Number); \
    STATIC_REQUIRE(extractor.tokens_[0].string_ == EXPECTED);                  \
  }

#define STATIC_REQUIRE_NUMBER_TEST_FAIL(NUMBER, MESSAGE)                     \
  {                                                                          \
    constexpr auto json = NUMBER;                                            \
    STATIC_REQUIRE_FALSE_COUNTER_AND_EXTRACTOR(                              \
        counter_error, extractor_error, json, parser::ConsumerType::Number); \
    STATIC_REQUIRE(counter_error.message_ == MESSAGE);                       \
    STATIC_REQUIRE(extractor_error.message_ == MESSAGE);                     \
  }

#define STATIC_REQUIRE_STRING_TEST_PASS(STRING, EXPECTED)                      \
  {                                                                            \
    constexpr auto json = STRING;                                              \
    STATIC_REQUIRE_COUNTER_AND_EXTRACTOR(                                      \
        counter, extractor, json, parser::ConsumerType::String);               \
    STATIC_REQUIRE(counter.count_ == 1);                                       \
    STATIC_REQUIRE(extractor.index_ == 1);                                     \
    STATIC_REQUIRE(extractor.tokens_[0].type_ == parser::Token::Type::String); \
    STATIC_REQUIRE(extractor.tokens_[0].string_ == EXPECTED);                  \
  }

#define STATIC_REQUIRE_STRING_TEST_FAIL(STRING, MESSAGE)                     \
  {                                                                          \
    constexpr auto json = STRING;                                            \
    STATIC_REQUIRE_FALSE_COUNTER_AND_EXTRACTOR(                              \
        counter_error, extractor_error, json, parser::ConsumerType::String); \
    STATIC_REQUIRE(counter_error.message_ == MESSAGE);                       \
    STATIC_REQUIRE(extractor_error.message_ == MESSAGE);                     \
  }

#define STATIC_REQUIRE_VALUE_TEST_PASS(VALUE, TYPE, EXPECTED)                \
  {                                                                          \
    constexpr auto json = VALUE;                                             \
    STATIC_REQUIRE_COUNTER_AND_EXTRACTOR(                                    \
        counter, extractor, json, parser::ConsumerType::Value);              \
    STATIC_REQUIRE(counter.count_ == 1);                                     \
    STATIC_REQUIRE(extractor.index_ == 1);                                   \
    STATIC_REQUIRE(extractor.tokens_[0].type_ == parser::Token::Type::TYPE); \
    STATIC_REQUIRE(extractor.tokens_[0].string_ == EXPECTED);                \
  }

#define STATIC_REQUIRE_VALUE_TEST_FAIL(VALUE, MESSAGE)                      \
  {                                                                         \
    constexpr auto json = VALUE;                                            \
    STATIC_REQUIRE_FALSE_COUNTER_AND_EXTRACTOR(                             \
        counter_error, extractor_error, json, parser::ConsumerType::Value); \
    STATIC_REQUIRE(counter_error.message_ == MESSAGE);                      \
    STATIC_REQUIRE(extractor_error.message_ == MESSAGE);                    \
  }

namespace injectx::json::tests {
struct TokenCounter {
  std::size_t count_ = 0;

  constexpr stdext::expected<void, parser::ParseError> operator()(
      parser::Token) {
    ++count_;
    return {};
  }
};

template<std::size_t N>
struct TokenExtractor {
  std::array<parser::Token, N> tokens_{};
  std::size_t index_ = 0;

  constexpr stdext::expected<void, parser::ParseError> operator()(
      parser::Token token) {
    if (index_ >= N) {
      return stdext::unexpected{
          parser::ParseError{.message_ = "Programming error"}};
    }
    tokens_[index_++] = token;

    return {};
  }
};

template<parser::ConsumerType consumerType, auto jsonStringCreator>
inline consteval auto getCounterAndExtractor() {
  constexpr auto counter_result = std::invoke([]() {
    TokenCounter counter;
    parser::JsonCursor cursor{.json = jsonStringCreator()};
    return parser::selectConsumeFunction<TokenCounter>(consumerType)(
               cursor, counter)
         | stdext::transform([&counter](auto) {
             return counter;
           });
  });
  constexpr auto counter =
      counter_result.has_value() ? *counter_result : TokenCounter{};
  constexpr auto extractor_result = std::invoke([&counter]() {
    using Extractor = TokenExtractor<counter.count_>;
    Extractor extractor;
    parser::JsonCursor cursor{.json = jsonStringCreator()};
    return parser::selectConsumeFunction<Extractor>(consumerType)(
               cursor, extractor)
         | stdext::transform([&extractor](auto) {
             return extractor;
           });
  });
  return std::pair{counter_result, extractor_result};
}

TEST_CASE("consumeNull") {
  SECTION("positive") {
    STATIC_REQUIRE_NULL_TEST_PASS("null");
    STATIC_REQUIRE_NULL_TEST_PASS("null  ");
    STATIC_REQUIRE_NULL_TEST_PASS("nullkdlsakl ");
    STATIC_REQUIRE_NULL_TEST_PASS("nullnull");
    STATIC_REQUIRE_NULL_TEST_PASS("null,");
  }
  SECTION("negative") {
    STATIC_REQUIRE_NULL_TEST_FAIL(
        "nul l", "Expected null but couldn't parse it");
    STATIC_REQUIRE_NULL_TEST_FAIL("nul", "Expected null but couldn't parse it");
    STATIC_REQUIRE_NULL_TEST_FAIL(
        " nullkdlsakl ", "Expected null but couldn't parse it");
    STATIC_REQUIRE_NULL_TEST_FAIL(
        "nnullnull", "Expected null but couldn't parse it");
    STATIC_REQUIRE_NULL_TEST_FAIL(
        ",null,", "Expected null but couldn't parse it");
  }
}

TEST_CASE("consumeBoolean") {
  SECTION("positive-exact-true") {
    STATIC_REQUIRE_BOOLEAN_TEST_PASS("true", "true");
    STATIC_REQUIRE_BOOLEAN_TEST_PASS("true  ", "true");
    STATIC_REQUIRE_BOOLEAN_TEST_PASS("truekdlsakl ", "true");
    STATIC_REQUIRE_BOOLEAN_TEST_PASS("truetrue", "true");
    STATIC_REQUIRE_BOOLEAN_TEST_PASS("true,", "true");
    STATIC_REQUIRE_BOOLEAN_TEST_PASS("false", "false");
    STATIC_REQUIRE_BOOLEAN_TEST_PASS("false  ", "false");
    STATIC_REQUIRE_BOOLEAN_TEST_PASS("falsekdlsakl ", "false");
    STATIC_REQUIRE_BOOLEAN_TEST_PASS("falsefalse", "false");
    STATIC_REQUIRE_BOOLEAN_TEST_PASS("false,", "false");
  }
  SECTION("negative") {
    STATIC_REQUIRE_BOOLEAN_TEST_FAIL(
        " true", "Expected a boolean but couldn't parse it");
    STATIC_REQUIRE_BOOLEAN_TEST_FAIL(
        "ttrue  ", "Expected a boolean but couldn't parse it");
    STATIC_REQUIRE_BOOLEAN_TEST_FAIL(
        "ftruekdlsakl ", "Expected a boolean but couldn't parse it");
    STATIC_REQUIRE_BOOLEAN_TEST_FAIL(
        "trutrue", "Expected a boolean but couldn't parse it");
    STATIC_REQUIRE_BOOLEAN_TEST_FAIL(
        "truue,", "Expected a boolean but couldn't parse it");
    STATIC_REQUIRE_BOOLEAN_TEST_FAIL(
        " false", "Expected a boolean but couldn't parse it");
    STATIC_REQUIRE_BOOLEAN_TEST_FAIL(
        "ffalse  ", "Expected a boolean but couldn't parse it");
    STATIC_REQUIRE_BOOLEAN_TEST_FAIL(
        "tfalsekdlsakl ", "Expected a boolean but couldn't parse it");
    STATIC_REQUIRE_BOOLEAN_TEST_FAIL(
        "falsfalse", "Expected a boolean but couldn't parse it");
    STATIC_REQUIRE_BOOLEAN_TEST_FAIL(
        "falsse,", "Expected a boolean but couldn't parse it");
  }
}

TEST_CASE("consumeNumber") {
  SECTION("positive-integer") {
    STATIC_REQUIRE_NUMBER_TEST_PASS("8934523", "8934523");
    STATIC_REQUIRE_NUMBER_TEST_PASS("-9053290 ", "-9053290");
    STATIC_REQUIRE_NUMBER_TEST_PASS("0\n", "0");
    STATIC_REQUIRE_NUMBER_TEST_PASS("-0%", "-0");
    STATIC_REQUIRE_NUMBER_TEST_PASS(
        "8934523995834958493589435849582930432589230859320890,",
        "8934523995834958493589435849582930432589230859320890");
    STATIC_REQUIRE_NUMBER_TEST_PASS(
        "-8934523995834958493589435849582930432589230859320890]",
        "-8934523995834958493589435849582930432589230859320890");
  }
  SECTION("positive-integer-with-fraction") {
    STATIC_REQUIRE_NUMBER_TEST_PASS(
        "654.932049320592095302}", "654.932049320592095302");
    STATIC_REQUIRE_NUMBER_TEST_PASS(
        "-905329532.395034859328{", "-905329532.395034859328");
    STATIC_REQUIRE_NUMBER_TEST_PASS("0.9538958293$", "0.9538958293");
    STATIC_REQUIRE_NUMBER_TEST_PASS("-0.8594358698239\t", "-0.8594358698239");
    STATIC_REQUIRE_NUMBER_TEST_PASS(
        "90543905960950350259023592309.905238592385293502359802\b",
        "90543905960950350259023592309.905238592385293502359802");
    STATIC_REQUIRE_NUMBER_TEST_PASS(
        "-90543905960950350259023592309.905238592385293502359802",
        "-90543905960950350259023592309.905238592385293502359802");
  }
  SECTION("positive-integer-with-exponent") {
    STATIC_REQUIRE_NUMBER_TEST_PASS("654e32", "654e32");
    STATIC_REQUIRE_NUMBER_TEST_PASS("-654e32", "-654e32");
    STATIC_REQUIRE_NUMBER_TEST_PASS("654e-32", "654e-32");
    STATIC_REQUIRE_NUMBER_TEST_PASS("-654e-32", "-654e-32");
    STATIC_REQUIRE_NUMBER_TEST_PASS("654e+32", "654e+32");
    STATIC_REQUIRE_NUMBER_TEST_PASS("-654e+32", "-654e+32");
    STATIC_REQUIRE_NUMBER_TEST_PASS("-654e+0", "-654e+0");
    STATIC_REQUIRE_NUMBER_TEST_PASS("-654e-0", "-654e-0");
    STATIC_REQUIRE_NUMBER_TEST_PASS("-0e-0", "-0e-0");
    STATIC_REQUIRE_NUMBER_TEST_PASS("-0e+0", "-0e+0");
    STATIC_REQUIRE_NUMBER_TEST_PASS("0e-0", "0e-0");
    STATIC_REQUIRE_NUMBER_TEST_PASS("0e+0", "0e+0");
    STATIC_REQUIRE_NUMBER_TEST_PASS("654E32", "654E32");
    STATIC_REQUIRE_NUMBER_TEST_PASS("-654E32", "-654E32");
    STATIC_REQUIRE_NUMBER_TEST_PASS("654E-32", "654E-32");
    STATIC_REQUIRE_NUMBER_TEST_PASS("-654E-32", "-654E-32");
    STATIC_REQUIRE_NUMBER_TEST_PASS("654E+32", "654E+32");
    STATIC_REQUIRE_NUMBER_TEST_PASS("-654E+32", "-654E+32");
    STATIC_REQUIRE_NUMBER_TEST_PASS("-654E+0", "-654E+0");
    STATIC_REQUIRE_NUMBER_TEST_PASS("-654E-0", "-654E-0");
    STATIC_REQUIRE_NUMBER_TEST_PASS("-0E-0", "-0E-0");
    STATIC_REQUIRE_NUMBER_TEST_PASS("-0E+0", "-0E+0");
    STATIC_REQUIRE_NUMBER_TEST_PASS("0E-0", "0E-0");
    STATIC_REQUIRE_NUMBER_TEST_PASS("0E+0", "0E+0");
  }
  SECTION("positive-integer-with-fraction-and-exponent") {
    STATIC_REQUIRE_NUMBER_TEST_PASS(
        "48394328.5049053e332", "48394328.5049053e332");
    STATIC_REQUIRE_NUMBER_TEST_PASS(
        "-48394328.5049053e32", "-48394328.5049053e32");
    STATIC_REQUIRE_NUMBER_TEST_PASS(
        "48394328.5049053e+38", "48394328.5049053e+38");
    STATIC_REQUIRE_NUMBER_TEST_PASS(
        "48394328.5049053e-38", "48394328.5049053e-38");
  }
  SECTION("negative") {
    STATIC_REQUIRE_NUMBER_TEST_FAIL("+954839593", "expected a digit or '-'");
    STATIC_REQUIRE_NUMBER_TEST_FAIL(
        "-.94032", "'-' should be followed by a digit");
    STATIC_REQUIRE_NUMBER_TEST_FAIL(
        "-0.e123", "'.' should be followed by a digit");
    STATIC_REQUIRE_NUMBER_TEST_FAIL(
        "-0.1Ea123", "'e(+/-)' or 'E(+/-)' should be followed by a digit");
    STATIC_REQUIRE_NUMBER_TEST_FAIL(
        "-0.1e+a123", "'e(+/-)' or 'E(+/-)' should be followed by a digit");
    STATIC_REQUIRE_NUMBER_TEST_FAIL(
        "-0.1E-a123", "'e(+/-)' or 'E(+/-)' should be followed by a digit");
  }
}

TEST_CASE("consumeString") {
  SECTION("positive") {
    STATIC_REQUIRE_STRING_TEST_PASS(R"("")", R"()");
    STATIC_REQUIRE_STRING_TEST_PASS(R"("8934523")", R"(8934523)");

    STATIC_REQUIRE_STRING_TEST_PASS(R"("\"8934523")", R"(\"8934523)");
    STATIC_REQUIRE_STRING_TEST_PASS(
        R"("\"89   lfewkl 34523" we are not interested in what happens after
            ending quote)",
        R"(\"89   lfewkl 34523)");
    STATIC_REQUIRE_STRING_TEST_PASS(
        R"("\\ \/ \b \f \r \t \n \u432AD" kbrejgiw)",
        R"(\\ \/ \b \f \r \t \n \u432AD)");
  }

  SECTION("negative") {
    STATIC_REQUIRE_STRING_TEST_FAIL(
        R"( 8934523)", "Strings must begin with '\"'");
    STATIC_REQUIRE_STRING_TEST_FAIL(
        R"("8934523)",
        "Reached to end while looking for an enclosing '\"' "
        "for the string");
    STATIC_REQUIRE_STRING_TEST_FAIL(
        R"("8934523\")",
        "Reached to end while looking for an enclosing '\"' "
        "for the string");
    STATIC_REQUIRE_STRING_TEST_FAIL(
        R"("\x")",
        "Backslashes should be followed by one of the"
        "escapable characters: \",b,f,n,r,t,u,\\,/");
    STATIC_REQUIRE_STRING_TEST_FAIL(
        R"("\v")",
        "Backslashes should be followed by one of the"
        "escapable characters: \",b,f,n,r,t,u,\\,/");
    STATIC_REQUIRE_STRING_TEST_FAIL(
        R"("\u")",
        "\\u should be followed by 4 hexadecimal digits "
        "for proper unicode escaping");
    STATIC_REQUIRE_STRING_TEST_FAIL(
        R"("\u123")",
        "\\u should be followed by 4 hexadecimal digits "
        "for proper unicode escaping");
    STATIC_REQUIRE_STRING_TEST_FAIL(
        R"("\u231G")",
        "\\u should be followed by 4 hexadecimal digits "
        "for proper unicode escaping");
    STATIC_REQUIRE_STRING_TEST_FAIL(
        R"("\u231x")",
        "\\u should be followed by 4 hexadecimal digits "
        "for proper unicode escaping");
  }
}

TEST_CASE("consumeValue") {
  SECTION("positive") {
    STATIC_REQUIRE_VALUE_TEST_PASS("null", Null, "null");
    STATIC_REQUIRE_VALUE_TEST_PASS("null  ", Null, "null");
    STATIC_REQUIRE_VALUE_TEST_PASS("nullkdlsakl ", Null, "null");
    STATIC_REQUIRE_VALUE_TEST_PASS("nullnull", Null, "null");
    STATIC_REQUIRE_VALUE_TEST_PASS("null,", Null, "null");
    STATIC_REQUIRE_VALUE_TEST_PASS("true", Boolean, "true");
    STATIC_REQUIRE_VALUE_TEST_PASS("true  ", Boolean, "true");
    STATIC_REQUIRE_VALUE_TEST_PASS("truekdlsakl ", Boolean, "true");
    STATIC_REQUIRE_VALUE_TEST_PASS("truetrue", Boolean, "true");
    STATIC_REQUIRE_VALUE_TEST_PASS("true,", Boolean, "true");
    STATIC_REQUIRE_VALUE_TEST_PASS("false", Boolean, "false");
    STATIC_REQUIRE_VALUE_TEST_PASS("false  ", Boolean, "false");
    STATIC_REQUIRE_VALUE_TEST_PASS("falsekdlsakl ", Boolean, "false");
    STATIC_REQUIRE_VALUE_TEST_PASS("falsefalse", Boolean, "false");
    STATIC_REQUIRE_VALUE_TEST_PASS("false,", Boolean, "false");
    STATIC_REQUIRE_VALUE_TEST_PASS("8934523", Number, "8934523");
    STATIC_REQUIRE_VALUE_TEST_PASS("-9053290 ", Number, "-9053290");
    STATIC_REQUIRE_VALUE_TEST_PASS("0\n", Number, "0");
    STATIC_REQUIRE_VALUE_TEST_PASS("-0%", Number, "-0");
    STATIC_REQUIRE_VALUE_TEST_PASS(
        "8934523995834958493589435849582930432589230859320890,", Number,
        "8934523995834958493589435849582930432589230859320890");
    STATIC_REQUIRE_VALUE_TEST_PASS(
        "-8934523995834958493589435849582930432589230859320890]", Number,
        "-8934523995834958493589435849582930432589230859320890");
    STATIC_REQUIRE_VALUE_TEST_PASS(
        "654.932049320592095302}", Number, "654.932049320592095302");
    STATIC_REQUIRE_VALUE_TEST_PASS(
        "-905329532.395034859328{", Number, "-905329532.395034859328");
    STATIC_REQUIRE_VALUE_TEST_PASS("0.9538958293$", Number, "0.9538958293");
    STATIC_REQUIRE_VALUE_TEST_PASS(
        "-0.8594358698239\t", Number, "-0.8594358698239");
    STATIC_REQUIRE_VALUE_TEST_PASS(
        "90543905960950350259023592309.905238592385293502359802\b", Number,
        "90543905960950350259023592309.905238592385293502359802");
    STATIC_REQUIRE_VALUE_TEST_PASS(
        "-90543905960950350259023592309.905238592385293502359802", Number,
        "-90543905960950350259023592309.905238592385293502359802");
    STATIC_REQUIRE_VALUE_TEST_PASS("654e32", Number, "654e32");
    STATIC_REQUIRE_VALUE_TEST_PASS("-654e32", Number, "-654e32");
    STATIC_REQUIRE_VALUE_TEST_PASS("654e-32", Number, "654e-32");
    STATIC_REQUIRE_VALUE_TEST_PASS("-654e-32", Number, "-654e-32");
    STATIC_REQUIRE_VALUE_TEST_PASS("654e+32", Number, "654e+32");
    STATIC_REQUIRE_VALUE_TEST_PASS("-654e+32", Number, "-654e+32");
    STATIC_REQUIRE_VALUE_TEST_PASS("-654e+0", Number, "-654e+0");
    STATIC_REQUIRE_VALUE_TEST_PASS("-654e-0", Number, "-654e-0");
    STATIC_REQUIRE_VALUE_TEST_PASS("-0e-0", Number, "-0e-0");
    STATIC_REQUIRE_VALUE_TEST_PASS("-0e+0", Number, "-0e+0");
    STATIC_REQUIRE_VALUE_TEST_PASS("0e-0", Number, "0e-0");
    STATIC_REQUIRE_VALUE_TEST_PASS("0e+0", Number, "0e+0");
    STATIC_REQUIRE_VALUE_TEST_PASS("654E32", Number, "654E32");
    STATIC_REQUIRE_VALUE_TEST_PASS("-654E32", Number, "-654E32");
    STATIC_REQUIRE_VALUE_TEST_PASS("654E-32", Number, "654E-32");
    STATIC_REQUIRE_VALUE_TEST_PASS("-654E-32", Number, "-654E-32");
    STATIC_REQUIRE_VALUE_TEST_PASS("654E+32", Number, "654E+32");
    STATIC_REQUIRE_VALUE_TEST_PASS("-654E+32", Number, "-654E+32");
    STATIC_REQUIRE_VALUE_TEST_PASS("-654E+0", Number, "-654E+0");
    STATIC_REQUIRE_VALUE_TEST_PASS("-654E-0", Number, "-654E-0");
    STATIC_REQUIRE_VALUE_TEST_PASS("-0E-0", Number, "-0E-0");
    STATIC_REQUIRE_VALUE_TEST_PASS("-0E+0", Number, "-0E+0");
    STATIC_REQUIRE_VALUE_TEST_PASS("0E-0", Number, "0E-0");
    STATIC_REQUIRE_VALUE_TEST_PASS("0E+0", Number, "0E+0");
    STATIC_REQUIRE_VALUE_TEST_PASS(
        "48394328.5049053e332", Number, "48394328.5049053e332");
    STATIC_REQUIRE_VALUE_TEST_PASS(
        "-48394328.5049053e32", Number, "-48394328.5049053e32");
    STATIC_REQUIRE_VALUE_TEST_PASS(
        "48394328.5049053e+38", Number, "48394328.5049053e+38");
    STATIC_REQUIRE_VALUE_TEST_PASS(
        "48394328.5049053e-38", Number, "48394328.5049053e-38");
    STATIC_REQUIRE_VALUE_TEST_PASS(R"("")", String, R"()");
    STATIC_REQUIRE_VALUE_TEST_PASS(R"("8934523")", String, R"(8934523)");

    STATIC_REQUIRE_VALUE_TEST_PASS(R"("\"8934523")", String, R"(\"8934523)");
    STATIC_REQUIRE_VALUE_TEST_PASS(
        R"("\"89   lfewkl 34523" we are not interested in what happens after
            ending quote)",
        String, R"(\"89   lfewkl 34523)");
    STATIC_REQUIRE_VALUE_TEST_PASS(
        R"("\\ \/ \b \f \r \t \n \u432AD" kbrejgiw)", String,
        R"(\\ \/ \b \f \r \t \n \u432AD)");
  }
  SECTION("negative") {
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        "nul l", "Expected null but couldn't parse it");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        "nul", "Expected null but couldn't parse it");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        " nullkdlsakl ",
        "Values can either be an object, an array, a string, a number, true, "
        "false or null");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        "nnullnull", "Expected null but couldn't parse it");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        ",null,",
        "Values can either be an object, an array, a string, a number, true, "
        "false or null");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        " true",
        "Values can either be an object, an array, a string, a number, true, "
        "false or null");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        "ttrue  ", "Expected a boolean but couldn't parse it");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        "ftruekdlsakl ", "Expected a boolean but couldn't parse it");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        "trutrue", "Expected a boolean but couldn't parse it");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        "truue,", "Expected a boolean but couldn't parse it");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        " false",
        "Values can either be an object, an array, a string, a number, true, "
        "false or null");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        "ffalse  ", "Expected a boolean but couldn't parse it");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        "tfalsekdlsakl ", "Expected a boolean but couldn't parse it");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        "falsfalse", "Expected a boolean but couldn't parse it");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        "falsse,", "Expected a boolean but couldn't parse it");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        "+954839593",
        "Values can either be an object, an array, a string, a number, true, "
        "false or null");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        "-.94032", "'-' should be followed by a digit");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        "-0.e123", "'.' should be followed by a digit");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        "-0.1Ea123", "'e(+/-)' or 'E(+/-)' should be followed by a digit");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        "-0.1e+a123", "'e(+/-)' or 'E(+/-)' should be followed by a digit");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        "-0.1E-a123", "'e(+/-)' or 'E(+/-)' should be followed by a digit");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        R"( 8934523)",
        "Values can either be an object, an array, a string, a number, true, "
        "false or null");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        R"("8934523)",
        "Reached to end while looking for an enclosing '\"' "
        "for the string");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        R"("8934523\")",
        "Reached to end while looking for an enclosing '\"' "
        "for the string");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        R"("\x")",
        "Backslashes should be followed by one of the"
        "escapable characters: \",b,f,n,r,t,u,\\,/");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        R"("\v")",
        "Backslashes should be followed by one of the"
        "escapable characters: \",b,f,n,r,t,u,\\,/");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        R"("\u")",
        "\\u should be followed by 4 hexadecimal digits "
        "for proper unicode escaping");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        R"("\u123")",
        "\\u should be followed by 4 hexadecimal digits "
        "for proper unicode escaping");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        R"("\u231G")",
        "\\u should be followed by 4 hexadecimal digits "
        "for proper unicode escaping");
    STATIC_REQUIRE_VALUE_TEST_FAIL(
        R"("\u231x")",
        "\\u should be followed by 4 hexadecimal digits "
        "for proper unicode escaping");
  }
}

TEST_CASE("consumeDocument") {
  SECTION("empty-document") {
    constexpr auto json = "";
    STATIC_REQUIRE_FALSE_COUNTER_AND_EXTRACTOR(
        counter_error, extractor_error, json, parser::ConsumerType::Document);
    STATIC_REQUIRE(counter_error.message_ == "Empty document");
    STATIC_REQUIRE(extractor_error.message_ == "Empty document");
  }

  SECTION("empty-object") {
    constexpr auto json = "{}";
    STATIC_REQUIRE_COUNTER_AND_EXTRACTOR(
        counter, extractor, json, parser::ConsumerType::Document);
    STATIC_REQUIRE(counter.count_ == 2);
    STATIC_REQUIRE(extractor.index_ == 2);
    STATIC_REQUIRE(
        extractor.tokens_[0].type_ == parser::Token::Type::ObjectBegin);
    STATIC_REQUIRE(extractor.tokens_[0].string_ == "{");
    STATIC_REQUIRE(
        extractor.tokens_[1].type_ == parser::Token::Type::ObjectEnd);
    STATIC_REQUIRE(extractor.tokens_[1].string_ == "}");
  }

  SECTION("empty-array") {
    constexpr auto json = "[]";
    STATIC_REQUIRE_COUNTER_AND_EXTRACTOR(
        counter, extractor, json, parser::ConsumerType::Document);
    STATIC_REQUIRE(counter.count_ == 2);
    STATIC_REQUIRE(extractor.index_ == 2);
    STATIC_REQUIRE(
        extractor.tokens_[0].type_ == parser::Token::Type::ArrayBegin);
    STATIC_REQUIRE(extractor.tokens_[0].string_ == "[");
    STATIC_REQUIRE(extractor.tokens_[1].type_ == parser::Token::Type::ArrayEnd);
    STATIC_REQUIRE(extractor.tokens_[1].string_ == "]");
  }

  SECTION("simple-object") {
    constexpr auto json = R"(
{
      "key" : "value"
}
)";
    STATIC_REQUIRE_COUNTER_AND_EXTRACTOR(
        counter, extractor, json, parser::ConsumerType::Document);
    STATIC_REQUIRE(counter.count_ == 4);
    STATIC_REQUIRE(extractor.index_ == 4);
    STATIC_REQUIRE(
        extractor.tokens_[0].type_ == parser::Token::Type::ObjectBegin);
    STATIC_REQUIRE(extractor.tokens_[0].string_ == "{");
    STATIC_REQUIRE(extractor.tokens_[1].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[1].string_ == "key");
    STATIC_REQUIRE(extractor.tokens_[2].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[2].string_ == "value");
    STATIC_REQUIRE(
        extractor.tokens_[3].type_ == parser::Token::Type::ObjectEnd);
    STATIC_REQUIRE(extractor.tokens_[3].string_ == "}");
  }

  SECTION("simple-array-of-strings") {
    constexpr auto json = R"(
[
      "Some", "strings", "here", "for", "a", "test",
      "unicode \u343A escape", "\b\f\n\r\t\""
]
)";
    STATIC_REQUIRE_COUNTER_AND_EXTRACTOR(
        counter, extractor, json, parser::ConsumerType::Document);
    STATIC_REQUIRE(counter.count_ == 10);
    STATIC_REQUIRE(extractor.index_ == 10);
    STATIC_REQUIRE(
        extractor.tokens_[0].type_ == parser::Token::Type::ArrayBegin);
    STATIC_REQUIRE(extractor.tokens_[0].string_ == "[");
    STATIC_REQUIRE(extractor.tokens_[1].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[1].string_ == "Some");
    STATIC_REQUIRE(extractor.tokens_[2].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[2].string_ == "strings");
    STATIC_REQUIRE(extractor.tokens_[3].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[3].string_ == "here");
    STATIC_REQUIRE(extractor.tokens_[4].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[4].string_ == "for");
    STATIC_REQUIRE(extractor.tokens_[5].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[5].string_ == "a");
    STATIC_REQUIRE(extractor.tokens_[6].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[6].string_ == "test");
    STATIC_REQUIRE(extractor.tokens_[7].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[7].string_ == R"(unicode \u343A escape)");
    STATIC_REQUIRE(extractor.tokens_[8].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[8].string_ == R"(\b\f\n\r\t\")");
    STATIC_REQUIRE(extractor.tokens_[9].type_ == parser::Token::Type::ArrayEnd);
    STATIC_REQUIRE(extractor.tokens_[9].string_ == "]");
  }

  SECTION("simple-array-of-numbers") {
    constexpr auto json = R"(
[
      1, 2.5, 0.2, 0e123, 5e-1, 10e+23,
      -1, -2.5, -0.2, -0e123, -5e-1, -10e+23
]
)";
    STATIC_REQUIRE_COUNTER_AND_EXTRACTOR(
        counter, extractor, json, parser::ConsumerType::Document);
    STATIC_REQUIRE(counter.count_ == 14);
    STATIC_REQUIRE(extractor.index_ == 14);
    STATIC_REQUIRE(
        extractor.tokens_[0].type_ == parser::Token::Type::ArrayBegin);
    STATIC_REQUIRE(extractor.tokens_[0].string_ == "[");
    STATIC_REQUIRE(extractor.tokens_[1].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[1].string_ == "1");
    STATIC_REQUIRE(extractor.tokens_[2].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[2].string_ == "2.5");
    STATIC_REQUIRE(extractor.tokens_[3].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[3].string_ == "0.2");
    STATIC_REQUIRE(extractor.tokens_[4].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[4].string_ == "0e123");
    STATIC_REQUIRE(extractor.tokens_[5].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[5].string_ == "5e-1");
    STATIC_REQUIRE(extractor.tokens_[6].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[6].string_ == "10e+23");
    STATIC_REQUIRE(extractor.tokens_[7].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[7].string_ == "-1");
    STATIC_REQUIRE(extractor.tokens_[8].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[8].string_ == "-2.5");
    STATIC_REQUIRE(extractor.tokens_[9].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[9].string_ == "-0.2");
    STATIC_REQUIRE(extractor.tokens_[10].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[10].string_ == "-0e123");
    STATIC_REQUIRE(extractor.tokens_[11].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[11].string_ == "-5e-1");
    STATIC_REQUIRE(extractor.tokens_[12].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[12].string_ == "-10e+23");
    STATIC_REQUIRE(
        extractor.tokens_[13].type_ == parser::Token::Type::ArrayEnd);
    STATIC_REQUIRE(extractor.tokens_[13].string_ == "]");
  }

  SECTION("simple-array-of-booleans-and-nulls") {
    constexpr auto json = R"(
      [
        true, null,       false, true, null,null, true,
        false,
                  false,
                                    false,
                 true,
        null
      ]
    )";
    STATIC_REQUIRE_COUNTER_AND_EXTRACTOR(
        counter, extractor, json, parser::ConsumerType::Document);
    STATIC_REQUIRE(counter.count_ == 14);
    STATIC_REQUIRE(extractor.index_ == 14);
    STATIC_REQUIRE(
        extractor.tokens_[0].type_ == parser::Token::Type::ArrayBegin);
    STATIC_REQUIRE(extractor.tokens_[0].string_ == "[");
    STATIC_REQUIRE(extractor.tokens_[1].type_ == parser::Token::Type::Boolean);
    STATIC_REQUIRE(extractor.tokens_[1].string_ == "true");
    STATIC_REQUIRE(extractor.tokens_[2].type_ == parser::Token::Type::Null);
    STATIC_REQUIRE(extractor.tokens_[2].string_ == "null");
    STATIC_REQUIRE(extractor.tokens_[3].type_ == parser::Token::Type::Boolean);
    STATIC_REQUIRE(extractor.tokens_[3].string_ == "false");
    STATIC_REQUIRE(extractor.tokens_[4].type_ == parser::Token::Type::Boolean);
    STATIC_REQUIRE(extractor.tokens_[4].string_ == "true");
    STATIC_REQUIRE(extractor.tokens_[5].type_ == parser::Token::Type::Null);
    STATIC_REQUIRE(extractor.tokens_[5].string_ == "null");
    STATIC_REQUIRE(extractor.tokens_[6].type_ == parser::Token::Type::Null);
    STATIC_REQUIRE(extractor.tokens_[6].string_ == "null");
    STATIC_REQUIRE(extractor.tokens_[7].type_ == parser::Token::Type::Boolean);
    STATIC_REQUIRE(extractor.tokens_[7].string_ == "true");
    STATIC_REQUIRE(extractor.tokens_[8].type_ == parser::Token::Type::Boolean);
    STATIC_REQUIRE(extractor.tokens_[8].string_ == "false");
    STATIC_REQUIRE(extractor.tokens_[9].type_ == parser::Token::Type::Boolean);
    STATIC_REQUIRE(extractor.tokens_[9].string_ == "false");
    STATIC_REQUIRE(extractor.tokens_[10].type_ == parser::Token::Type::Boolean);
    STATIC_REQUIRE(extractor.tokens_[10].string_ == "false");
    STATIC_REQUIRE(extractor.tokens_[11].type_ == parser::Token::Type::Boolean);
    STATIC_REQUIRE(extractor.tokens_[11].string_ == "true");
    STATIC_REQUIRE(extractor.tokens_[12].type_ == parser::Token::Type::Null);
    STATIC_REQUIRE(extractor.tokens_[12].string_ == "null");
    STATIC_REQUIRE(
        extractor.tokens_[13].type_ == parser::Token::Type::ArrayEnd);
    STATIC_REQUIRE(extractor.tokens_[13].string_ == "]");
  }

  SECTION("vulnerable-cases") {
    constexpr auto json = R"(
      [
    [], {}, [], [{}]
      ]
    )";
    STATIC_REQUIRE_COUNTER_AND_EXTRACTOR(
        counter, extractor, json, parser::ConsumerType::Document);
    STATIC_REQUIRE(counter.count_ == 12);
    STATIC_REQUIRE(extractor.index_ == 12);
    STATIC_REQUIRE(
        extractor.tokens_[0].type_ == parser::Token::Type::ArrayBegin);
    STATIC_REQUIRE(extractor.tokens_[0].string_ == "[");
    STATIC_REQUIRE(
        extractor.tokens_[1].type_ == parser::Token::Type::ArrayBegin);
    STATIC_REQUIRE(extractor.tokens_[1].string_ == "[");
    STATIC_REQUIRE(extractor.tokens_[2].type_ == parser::Token::Type::ArrayEnd);
    STATIC_REQUIRE(extractor.tokens_[2].string_ == "]");
    STATIC_REQUIRE(
        extractor.tokens_[3].type_ == parser::Token::Type::ObjectBegin);
    STATIC_REQUIRE(extractor.tokens_[3].string_ == "{");
    STATIC_REQUIRE(
        extractor.tokens_[4].type_ == parser::Token::Type::ObjectEnd);
    STATIC_REQUIRE(extractor.tokens_[4].string_ == "}");
    STATIC_REQUIRE(
        extractor.tokens_[5].type_ == parser::Token::Type::ArrayBegin);
    STATIC_REQUIRE(extractor.tokens_[5].string_ == "[");
    STATIC_REQUIRE(extractor.tokens_[6].type_ == parser::Token::Type::ArrayEnd);
    STATIC_REQUIRE(extractor.tokens_[6].string_ == "]");
    STATIC_REQUIRE(
        extractor.tokens_[7].type_ == parser::Token::Type::ArrayBegin);
    STATIC_REQUIRE(extractor.tokens_[7].string_ == "[");
    STATIC_REQUIRE(
        extractor.tokens_[8].type_ == parser::Token::Type::ObjectBegin);
    STATIC_REQUIRE(extractor.tokens_[8].string_ == "{");
    STATIC_REQUIRE(
        extractor.tokens_[9].type_ == parser::Token::Type::ObjectEnd);
    STATIC_REQUIRE(extractor.tokens_[9].string_ == "}");
    STATIC_REQUIRE(
        extractor.tokens_[10].type_ == parser::Token::Type::ArrayEnd);
    STATIC_REQUIRE(extractor.tokens_[10].string_ == "]");
    STATIC_REQUIRE(
        extractor.tokens_[11].type_ == parser::Token::Type::ArrayEnd);
    STATIC_REQUIRE(extractor.tokens_[11].string_ == "]");
  }

  SECTION("Everything in an object") {
    constexpr auto json = R"({
      "string": "string",
      "number1": 19059043,
      "number2": 0.439253095,
      "number3": -0.00034230,
      "number4": 0.439253095e+56,
      "number5": -0.00034230e56,
      "number6": -0.00034230e-56,
      "number7": 140932e+56,
      "number8": 140932e56,
      "number9": 140932e-56,
      "null": null,
      "true": true,
      "false": false,
      "array" : [
        "string", 3.14, null, true, false, {}, []
      ]
  })";

    STATIC_REQUIRE_COUNTER_AND_EXTRACTOR(
        counter, extractor, json, parser::ConsumerType::Document);
    STATIC_REQUIRE(counter.count_ == 40);
    STATIC_REQUIRE(extractor.index_ == 40);
    STATIC_REQUIRE(
        extractor.tokens_[0].type_ == parser::Token::Type::ObjectBegin);
    STATIC_REQUIRE(extractor.tokens_[0].string_ == "{");
    STATIC_REQUIRE(extractor.tokens_[1].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[1].string_ == "string");
    STATIC_REQUIRE(extractor.tokens_[2].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[2].string_ == "string");
    STATIC_REQUIRE(extractor.tokens_[3].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[3].string_ == "number1");
    STATIC_REQUIRE(extractor.tokens_[4].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[4].string_ == "19059043");
    STATIC_REQUIRE(extractor.tokens_[5].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[5].string_ == "number2");
    STATIC_REQUIRE(extractor.tokens_[6].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[6].string_ == "0.439253095");
    STATIC_REQUIRE(extractor.tokens_[7].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[7].string_ == "number3");
    STATIC_REQUIRE(extractor.tokens_[8].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[8].string_ == "-0.00034230");
    STATIC_REQUIRE(extractor.tokens_[9].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[9].string_ == "number4");
    STATIC_REQUIRE(extractor.tokens_[10].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[10].string_ == "0.439253095e+56");
    STATIC_REQUIRE(extractor.tokens_[11].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[11].string_ == "number5");
    STATIC_REQUIRE(extractor.tokens_[12].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[12].string_ == "-0.00034230e56");
    STATIC_REQUIRE(extractor.tokens_[13].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[13].string_ == "number6");
    STATIC_REQUIRE(extractor.tokens_[14].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[14].string_ == "-0.00034230e-56");
    STATIC_REQUIRE(extractor.tokens_[15].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[15].string_ == "number7");
    STATIC_REQUIRE(extractor.tokens_[16].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[16].string_ == "140932e+56");
    STATIC_REQUIRE(extractor.tokens_[17].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[17].string_ == "number8");
    STATIC_REQUIRE(extractor.tokens_[18].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[18].string_ == "140932e56");
    STATIC_REQUIRE(extractor.tokens_[19].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[19].string_ == "number9");
    STATIC_REQUIRE(extractor.tokens_[20].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[20].string_ == "140932e-56");
    STATIC_REQUIRE(extractor.tokens_[21].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[21].string_ == "null");
    STATIC_REQUIRE(extractor.tokens_[22].type_ == parser::Token::Type::Null);
    STATIC_REQUIRE(extractor.tokens_[22].string_ == "null");
    STATIC_REQUIRE(extractor.tokens_[23].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[23].string_ == "true");
    STATIC_REQUIRE(extractor.tokens_[24].type_ == parser::Token::Type::Boolean);
    STATIC_REQUIRE(extractor.tokens_[24].string_ == "true");
    STATIC_REQUIRE(extractor.tokens_[25].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[25].string_ == "false");
    STATIC_REQUIRE(extractor.tokens_[26].type_ == parser::Token::Type::Boolean);
    STATIC_REQUIRE(extractor.tokens_[26].string_ == "false");
    STATIC_REQUIRE(extractor.tokens_[27].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[27].string_ == "array");
    STATIC_REQUIRE(
        extractor.tokens_[28].type_ == parser::Token::Type::ArrayBegin);
    STATIC_REQUIRE(extractor.tokens_[28].string_ == "[");
    STATIC_REQUIRE(extractor.tokens_[29].type_ == parser::Token::Type::String);
    STATIC_REQUIRE(extractor.tokens_[29].string_ == "string");
    STATIC_REQUIRE(extractor.tokens_[30].type_ == parser::Token::Type::Number);
    STATIC_REQUIRE(extractor.tokens_[30].string_ == "3.14");
    STATIC_REQUIRE(extractor.tokens_[31].type_ == parser::Token::Type::Null);
    STATIC_REQUIRE(extractor.tokens_[31].string_ == "null");
    STATIC_REQUIRE(extractor.tokens_[32].type_ == parser::Token::Type::Boolean);
    STATIC_REQUIRE(extractor.tokens_[32].string_ == "true");
    STATIC_REQUIRE(extractor.tokens_[33].type_ == parser::Token::Type::Boolean);
    STATIC_REQUIRE(extractor.tokens_[33].string_ == "false");
    STATIC_REQUIRE(
        extractor.tokens_[34].type_ == parser::Token::Type::ObjectBegin);
    STATIC_REQUIRE(extractor.tokens_[34].string_ == "{");
    STATIC_REQUIRE(
        extractor.tokens_[35].type_ == parser::Token::Type::ObjectEnd);
    STATIC_REQUIRE(extractor.tokens_[35].string_ == "}");
    STATIC_REQUIRE(
        extractor.tokens_[36].type_ == parser::Token::Type::ArrayBegin);
    STATIC_REQUIRE(extractor.tokens_[36].string_ == "[");
    STATIC_REQUIRE(
        extractor.tokens_[37].type_ == parser::Token::Type::ArrayEnd);
    STATIC_REQUIRE(extractor.tokens_[37].string_ == "]");
    STATIC_REQUIRE(
        extractor.tokens_[38].type_ == parser::Token::Type::ArrayEnd);
    STATIC_REQUIRE(extractor.tokens_[38].string_ == "]");
    STATIC_REQUIRE(
        extractor.tokens_[39].type_ == parser::Token::Type::ObjectEnd);
    STATIC_REQUIRE(extractor.tokens_[39].string_ == "}");
  }
}

}  // namespace injectx::json::tests