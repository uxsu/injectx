#pragma once

#include <algorithm>
#include <array>
#include <injectx/json/parser/token.hpp>
#include <injectx/json/parser/utility.hpp>
#include <injectx/stdext/expected.hpp>
#include <vector>

namespace injectx::json::parser {

struct ParseError {
  std::string_view message_;
  std::size_t parse_begin_ = 0;
  std::size_t parse_end_ = 0;
  Token::Type token_type_ = Token::Type::Invalid;
};

struct JsonCursor {
  std::string_view json;
  std::size_t current = 0;
  std::size_t marked = 0;
};

using CursorOrError = stdext::expected<JsonCursor, ParseError>;

namespace details::_parser {

inline constexpr char peek(JsonCursor cursor) {
  return cursor.json[cursor.current];
}

inline constexpr void advance(JsonCursor& cursor, std::size_t step = 1) {
  cursor.current = cursor.current + step;
}

inline constexpr std::size_t size(JsonCursor cursor) {
  return cursor.json.size();
}

inline constexpr bool canRead(JsonCursor cursor) {
  return cursor.current < size(cursor);
}

inline constexpr bool isNext(JsonCursor cursor, std::string_view str) {
  if (cursor.current + str.size() > size(cursor)) {
    return false;
  }
  return std::string_view(cursor.json.data() + cursor.current, str.size())
      == str;
}

inline constexpr JsonCursor mark(JsonCursor cursor) {
  cursor.marked = cursor.current;
  return cursor;
}

inline constexpr std::string_view fromMarked(
    JsonCursor cursor, std::size_t markOffset, std::size_t currentOffset) {
  return cursor.json.substr(
      cursor.marked + markOffset,
      cursor.current - currentOffset - cursor.marked - markOffset);
}

struct ErrorInfo {
  std::string_view message = "";
  Token::Type type = Token::Type::Invalid;

  inline constexpr auto makeError(JsonCursor cursor) const {
    return stdext::unexpected{ParseError{
        .message_ = message,
        .parse_begin_ = cursor.marked,
        .parse_end_ = cursor.current,
        .token_type_ = type}};
  }
};

template<typename Predicate>
struct MayAdvance {
  Predicate predicate;
  std::size_t steps = std::numeric_limits<std::size_t>::max();
  bool reachingEndAsError = false;
  ErrorInfo errorInfo{};

  inline constexpr CursorOrError operator()(JsonCursor cursor) {
    std::size_t step = 0;
    while (step < steps && canRead(cursor) && predicate(peek(cursor))) {
      advance(cursor);
      ++steps;
    }
    if (reachingEndAsError && !canRead(cursor)) {
      return errorInfo.makeError(cursor);
    }
    return cursor;
  }
};

template<typename Predicate>
struct ShouldAdvance {
  Predicate predicate;
  std::size_t steps = 1;
  ErrorInfo errorInfo{};

  inline constexpr CursorOrError operator()(JsonCursor cursor) {
    if (cursor.current + steps > size(cursor)) {
      return errorInfo.makeError(cursor);
    }
    std::size_t step = 0;
    while (step < steps) {
      if (!predicate(peek(cursor))) {
        return errorInfo.makeError(cursor);
      }
      ++step;
      advance(cursor);
    }
    return cursor;
  }
};

template<typename Predicate>
struct AdvanceWhile {
  Predicate predicate;
  bool reachingEndAsError = true;
  ErrorInfo errorInfo{};

  inline constexpr CursorOrError operator()(JsonCursor cursor) {
    while (canRead(cursor) && predicate(peek(cursor))) {
      advance(cursor);
    }
    if (reachingEndAsError && !canRead(cursor)) {
      return errorInfo.makeError(cursor);
    }
    return cursor;
  }
};

inline constexpr CursorOrError shouldSkipIntegerPart(JsonCursor cursor) {
  if (utility::isMinus(peek(cursor))) {
    advance(cursor);
  }
  if (utility::isZero(peek(cursor))) {
    advance(cursor);
    return cursor;
  }
  return AdvanceWhile{
      .predicate = utility::isDigit,
      .errorInfo = {
          .message = "'-' should be followed by a digit.",
          .type = Token::Type::Number}}(cursor);
}

inline constexpr CursorOrError tryAdvanceFractionalPart(JsonCursor cursor) {
  if (!utility::isDot(peek(cursor))) {
    return cursor;
  }
  advance(cursor);
  return CursorOrError(cursor)
       | stdext::and_then(ShouldAdvance{
           .predicate = utility::isDigit,
           .errorInfo =
               {.message = "'.' should be followed by a digit",
                .type = Token::Type::Number}})
       | stdext::and_then(AdvanceWhile{
           .predicate = utility::isDigit,
           .errorInfo = {
               .message = "Reached to end while parsing a number",
               .type = Token::Type::Number}});
}

inline constexpr CursorOrError tryAdvanceExponentialPart(JsonCursor cursor) {
  if (!utility::isExponent(peek(cursor))) {
    return cursor;
  }
  advance(cursor);
  return CursorOrError(cursor)
       | stdext::and_then(MayAdvance{
           .predicate = utility::isPlus || utility::isMinus, .steps = 1})
       | stdext::and_then(ShouldAdvance{
           .predicate = utility::isDigit,
           .errorInfo =
               {.message = "'e(+/-)' or 'E(+/-)' should be followed by a digit",
                .type = Token::Type::Number}})
       | stdext::and_then(AdvanceWhile{
           .predicate = utility::isDigit,
           .errorInfo = {
               .message = "Reached to end while parsing a number",
               .type = Token::Type::Number}});
}

inline constexpr CursorOrError advanceBoolean(JsonCursor cursor) {
  mark(cursor);
  if (isNext(cursor, "true")) {
    advance(cursor, 4);
    return cursor;
  } else if (isNext(cursor, "false")) {
    advance(cursor, 5);
    return cursor;
  }
  return ErrorInfo{
      .message = "Expected a boolean but couldn't parse it",
      .type = Token::Type::Boolean}
      .makeError(cursor);
}

inline constexpr CursorOrError advanceNull(JsonCursor cursor) {
  mark(cursor);
  if (isNext(cursor, "null")) {
    advance(cursor, 4);
    return cursor;
  }
  return ErrorInfo{
      .message = "Expected null but couldn't parse it",
      .type = Token::Type::Null}
      .makeError(cursor);
}

}  // namespace details::_parser

template<typename T>
concept Consumer = requires(T t, Token token) {
  { t.operator()(token) } -> std::same_as<stdext::expected<void, ParseError>>;
};

// declarations needed since consumeValue is used in consumeArrayElements
// and consumeKeyValuePairs it is neater to declare all consume* functions
// together
inline constexpr CursorOrError consumeDocument(JsonCursor, Consumer auto&);

inline constexpr CursorOrError consumeObject(JsonCursor, Consumer auto&);

inline constexpr CursorOrError consumeKeyValuePairs(JsonCursor, Consumer auto&);

inline constexpr CursorOrError consumeArray(JsonCursor, Consumer auto&);

inline constexpr CursorOrError consumeArrayElements(JsonCursor, Consumer auto&);

inline constexpr CursorOrError consumeValue(JsonCursor, Consumer auto&);

inline constexpr CursorOrError consumeString(JsonCursor, Consumer auto&);

inline constexpr CursorOrError consumeNumber(JsonCursor, Consumer auto&);

inline constexpr CursorOrError consumeBoolean(JsonCursor, Consumer auto&);

inline constexpr CursorOrError consumeNull(JsonCursor, Consumer auto&);

enum class ConsumerType {
  Document,
  Object,
  KeyValues,
  Array,
  Elements,
  Value,
  String,
  Number,
  Boolean,
  Null
};
template<Consumer StreamConsumer>
using ConsumeFunction = CursorOrError (*)(JsonCursor, StreamConsumer&);

template<Consumer StreamConsumer>
static constexpr auto ConsumeFunctions = std::array{
    std::pair<ConsumerType, ConsumeFunction<StreamConsumer>>{
        ConsumerType::Document, &consumeDocument<StreamConsumer>},
    std::pair<ConsumerType, ConsumeFunction<StreamConsumer>>{
        ConsumerType::Object, &consumeObject<StreamConsumer>},
    std::pair<ConsumerType, ConsumeFunction<StreamConsumer>>{
        ConsumerType::KeyValues, &consumeKeyValuePairs<StreamConsumer>},
    std::pair<ConsumerType, ConsumeFunction<StreamConsumer>>{
        ConsumerType::Array, &consumeArray<StreamConsumer>},
    std::pair<ConsumerType, ConsumeFunction<StreamConsumer>>{
        ConsumerType::Elements, &consumeArrayElements<StreamConsumer>},
    std::pair<ConsumerType, ConsumeFunction<StreamConsumer>>{
        ConsumerType::Value, &consumeValue<StreamConsumer>},
    std::pair<ConsumerType, ConsumeFunction<StreamConsumer>>{
        ConsumerType::String, &consumeString<StreamConsumer>},
    std::pair<ConsumerType, ConsumeFunction<StreamConsumer>>{
        ConsumerType::Number, &consumeNumber<StreamConsumer>},
    std::pair<ConsumerType, ConsumeFunction<StreamConsumer>>{
        ConsumerType::Boolean, &consumeBoolean<StreamConsumer>},
    std::pair<ConsumerType, ConsumeFunction<StreamConsumer>>{
        ConsumerType::Null, &consumeNull<StreamConsumer>},
};

template<Consumer StreamConsumer>
constexpr ConsumeFunction<StreamConsumer> selectConsumeFunction(
    ConsumerType type) {
  for (const auto& [t, f] : ConsumeFunctions<StreamConsumer>) {
    if (t == type) {
      return f;
    }
  }
  return ConsumeFunctions<StreamConsumer>[0].second;
}

namespace details::_parser {
inline constexpr bool canConsume(JsonCursor cursor, ConsumerType type) {
  char c = peek(cursor);
  switch (type) {
    case ConsumerType::Document:
      return canConsume(cursor, ConsumerType::Object)
          || canConsume(cursor, ConsumerType::Array);
    case ConsumerType::Object:
      return utility::isLeftBrace(c);
    case ConsumerType::KeyValues:
      return canConsume(cursor, ConsumerType::String)
          || utility::isRightBrace(c);
    case ConsumerType::Array:
      return utility::isLeftBracket(c);
    case ConsumerType::Elements:
      return canConsume(cursor, ConsumerType::Value)
          || utility::isRightBracket(c);
    case ConsumerType::Value:
      return canConsume(cursor, ConsumerType::String)
          || canConsume(cursor, ConsumerType::Number)
          || canConsume(cursor, ConsumerType::Boolean)
          || canConsume(cursor, ConsumerType::Null);
    case ConsumerType::String:
      return utility::isQuote(c);
    case ConsumerType::Number:
      return utility::canNumberStartWith(c);
    case ConsumerType::Boolean:
      return utility::canBooleanStartWith(c);
    case ConsumerType::Null:
      return utility::canNullStartWith(c);
    default:
      return false;
  }
}

template<Consumer StreamConsumer>
struct SelectConsumer {
  StreamConsumer& consumer;
  std::vector<ConsumerType> allowedConsumerTypes{};
  ErrorInfo errorInfo{};

  inline constexpr CursorOrError operator()(JsonCursor cursor) {
    for (auto type : allowedConsumerTypes) {
      if (canConsume(cursor, type)) {
        return selectConsumeFunction<StreamConsumer>(type)(cursor, consumer);
      }
    }
    return errorInfo.makeError(cursor);
  }
};

template<Consumer StreamConsumer>
struct DispatchToConsumer {
  StreamConsumer& consumer;
  ConsumerType allowedConsumerType;

  inline constexpr CursorOrError operator()(JsonCursor cursor) {
    auto consumerFunction =
        selectConsumeFunction<StreamConsumer>(allowedConsumerType);
    return consumerFunction(cursor, consumer);
  }
};

template<Consumer StreamConsumer>
struct Consume {
  StreamConsumer& consumer;
  Token::Type type = Token::Type::Invalid;
  std::size_t markOffset = 0;
  std::size_t currentOffset = 0;

  inline constexpr CursorOrError operator()(JsonCursor cursor) {
    return consumer(Token{
               .string_ = fromMarked(cursor, markOffset, currentOffset),
               .type_ = type})
         | stdext::transform([cursor]() {
             return cursor;
           });
  }
};

}  // namespace details::_parser

constexpr CursorOrError consumeDocument(
    JsonCursor cursor, Consumer auto& consumer) {
  using namespace details::_parser;
  return CursorOrError(cursor)
       | stdext::and_then(AdvanceWhile{
           .predicate = utility::isWhitespace,
           .errorInfo =
               {.message = "Empty document", .type = Token::Type::Invalid}})
       | stdext::transform(&mark)
       | stdext::and_then(SelectConsumer{
           .consumer = consumer,
           .allowedConsumerTypes = {ConsumerType::Object, ConsumerType::Array},
           .errorInfo = {
               .message = "Document must start with an object or an array",
               .type = Token::Type::Invalid}});
}

constexpr CursorOrError consumeObject(
    JsonCursor cursor, Consumer auto& consumer) {
  using namespace details::_parser;
  return CursorOrError(cursor) | stdext::transform(&mark)
       | stdext::and_then(ShouldAdvance{
           .predicate = utility::isLeftBrace,
           .errorInfo =
               {.message = "Objects must begin with '{'",
                .type = Token::Type::ObjectBegin}})
       | stdext::and_then(
             Consume{.consumer = consumer, .type = Token::Type::ObjectBegin})
       | stdext::and_then(AdvanceWhile{
           .predicate = utility::isWhitespace,
           .errorInfo =
               {.message = "Objects must end with '}'",
                .type = Token::Type::ObjectEnd}})
       | stdext::transform(&mark)
       | stdext::and_then(DispatchToConsumer{
           .consumer = consumer,
           .allowedConsumerType = ConsumerType::KeyValues,
       });
}

constexpr CursorOrError consumeKeyValuePairs(
    JsonCursor cursor, Consumer auto& consumer) {
  using namespace details::_parser;
  auto cur = CursorOrError(cursor)
           | stdext::and_then(AdvanceWhile{
               .predicate = utility::isWhitespace,
               .errorInfo = {
                   .message = "Reached to end while looking for a key",
                   .type = Token::Type::String}});
  if (!cur.has_value()) {
    return cur;
  }
  {
    auto curEnd =
        cur | stdext::transform(&mark)
        | stdext::and_then(ShouldAdvance{.predicate = utility::isRightBrace});
    if (curEnd.has_value()) {
      return curEnd
           | stdext::and_then(Consume{
               .consumer = consumer,
               .type = Token::Type::ObjectEnd,
           });
    }
  }
  auto curCont =
      cur | stdext::transform(&mark)
      | stdext::and_then(ShouldAdvance{
          .predicate = utility::isQuote,
          .errorInfo =
              {.message =
                   "Keys must be a string and thus must start with '\"' ",
               .type = Token::Type::String}})
      | stdext::transform(&mark)
      | stdext::and_then(DispatchToConsumer{
          .consumer = consumer, .allowedConsumerType = ConsumerType::String})
      | stdext::and_then(AdvanceWhile{
          .predicate = utility::isWhitespace,
          .errorInfo =
              {.message = "Reached to end while looking for colon",
               .type = Token::Type::String}})
      | stdext::transform(&mark)
      | stdext::and_then(ShouldAdvance{
          .predicate = utility::isColon,
          .errorInfo =
              {.message = "Keys must be separated by ':' from values",
               .type = Token::Type::Invalid}})
      | stdext::transform(&mark)
      | stdext::and_then(AdvanceWhile{
          .predicate = utility::isWhitespace,
          .errorInfo =
              {.message = "Reached to end while looking for a value",
               .type = Token::Type::String}})
      | stdext::transform(&mark)
      | stdext::and_then(DispatchToConsumer{
          .consumer = consumer, .allowedConsumerType = ConsumerType::Value})
      | stdext::and_then(AdvanceWhile{
          .predicate = utility::isWhitespace,
          .errorInfo = {
              .message = "Reached to end while looking for the end of object "
                         "or a comma to separate key-value pairs",
              .type = Token::Type::Invalid}});
  if (!curCont.has_value()) {
    return curCont;
  }
  auto curEnd = curCont | stdext::transform(&mark)
              | stdext::and_then(ShouldAdvance{
                  .predicate = utility::isRightBrace,
                  .steps = 1,
                  .errorInfo = ErrorInfo{}});
  if (curEnd.has_value()) {
    return curEnd
         | stdext::and_then(Consume{
             .consumer = consumer,
             .type = Token::Type::ObjectEnd,
         });
  }
  auto ret = curCont
           | stdext::and_then(ShouldAdvance{
               .predicate = utility::isComma,
               .errorInfo =
                   ErrorInfo{
                       .message = "Key-value pairs must be a separated by ','",
                       .type = Token::Type::Invalid}})
           | stdext::transform(&mark)
           | stdext::and_then(DispatchToConsumer{
               .consumer = consumer,
               .allowedConsumerType = ConsumerType::KeyValues});
  return ret;
}

constexpr CursorOrError consumeArray(
    JsonCursor cursor, Consumer auto& consumer) {
  using namespace details::_parser;
  return CursorOrError(cursor) | stdext::transform(&mark)
       | stdext::and_then(ShouldAdvance{
           .predicate = utility::isLeftBracket,
           .errorInfo =
               {.message = "Arrays must begin with '['",
                .type = Token::Type::ArrayBegin}})
       | stdext::and_then(
             Consume{.consumer = consumer, .type = Token::Type::ArrayBegin})
       | stdext::and_then(AdvanceWhile{
           .predicate = utility::isWhitespace,
           .errorInfo =
               {.message = "Arrays must end with '}'",
                .type = Token::Type::ArrayEnd}})
       | stdext::transform(&mark)
       | stdext::and_then(DispatchToConsumer{
           .consumer = consumer,
           .allowedConsumerType = ConsumerType::Elements});
}

constexpr CursorOrError consumeArrayElements(
    JsonCursor cursor, Consumer auto& consumer) {
  using namespace details::_parser;
  auto cur = CursorOrError(cursor)
           | stdext::and_then(AdvanceWhile{
               .predicate = utility::isWhitespace,
               .errorInfo = {
                   .message = "Reached to end while looking for a value ",
                   .type = Token::Type::Invalid}});
  if (!cur.has_value()) {
    return cur;
  }
  {
    auto curEnd =
        cur | stdext::transform(&mark)
        | stdext::and_then(ShouldAdvance{.predicate = utility::isRightBracket});
    if (curEnd.has_value()) {
      return curEnd
           | stdext::and_then(Consume{
               .consumer = consumer,
               .type = Token::Type::ArrayEnd,
           });
    }
  }
  auto curCont =
      cur | stdext::transform(&mark)
      | stdext::and_then(DispatchToConsumer{
          .consumer = consumer, .allowedConsumerType = ConsumerType::Value})
      | stdext::and_then(AdvanceWhile{
          .predicate = utility::isWhitespace,
          .reachingEndAsError = true,
          .errorInfo = {
              .message =
                  "Reached to end while looking for comma or end of array ",
              .type = Token::Type::Invalid}});
  {
    auto curEnd =
        curCont | stdext::transform(&mark)
        | stdext::and_then(ShouldAdvance{.predicate = utility::isRightBracket});
    if (curEnd.has_value()) {
      return curEnd
           | stdext::and_then(Consume{
               .consumer = consumer,
               .type = Token::Type::ArrayEnd,
           });
    }
  }
  return curCont
       | stdext::and_then(ShouldAdvance{
           .predicate = utility::isComma,
           .errorInfo =
               {.message = "Values must be a separated by ','",
                .type = Token::Type::Invalid}})
       | stdext::transform(&mark)
       | stdext::and_then(DispatchToConsumer{
           .consumer = consumer,
           .allowedConsumerType = ConsumerType::Elements});
}

constexpr CursorOrError consumeValue(
    JsonCursor cursor, Consumer auto& consumer) {
  using namespace details::_parser;
  auto cur = CursorOrError(cursor);
  auto curString =
      cur | stdext::and_then(ShouldAdvance{.predicate = utility::isQuote});
  if (curString.has_value()) {
    return curString | stdext::transform(&mark)
         | stdext::and_then(DispatchToConsumer{
             .consumer = consumer,
             .allowedConsumerType = ConsumerType::String});
  }

  return cur | stdext::transform(&mark)
       | stdext::and_then(SelectConsumer{
           .consumer = consumer,
           .allowedConsumerTypes =
               {ConsumerType::Object, ConsumerType::Array, ConsumerType::Number,
                ConsumerType::Boolean, ConsumerType::Null},
           .errorInfo = {
               .message = "Values can either be an object, an array, a string, "
                          "a number, true, false or null",
               .type = Token::Type::ArrayEnd}});
}

constexpr CursorOrError consumeString(
    JsonCursor cursor, Consumer auto& consumer) {
  using namespace details::_parser;
  auto cur =
      CursorOrError(cursor)
      | stdext::and_then(AdvanceWhile{
          .predicate = !(utility::isQuote || utility::isBackslash),
          .reachingEndAsError = true,
          .errorInfo = {
              .message = "Reached to end while looking for an enclosing '\"' "
                         "for the string",
              .type = Token::Type::String}});
  if (!cur.has_value()) {
    return cur;
  }
  {
    auto curEnd =
        cur | stdext::and_then(ShouldAdvance{.predicate = utility::isQuote});
    if (curEnd.has_value()) {
      return curEnd
           | stdext::and_then(Consume{
               .consumer = consumer,
               .type = Token::Type::String,
               .currentOffset = 1  // avoids the quote at the end
           });
    }
  }
  auto curCont =
      cur
      | stdext::and_then(ShouldAdvance{
          .predicate = utility::isBackslash,
          .steps = 1,
          .errorInfo = ErrorInfo{
              .message =
                  "Programming error, we shouldn't have an error here "}});
  auto curEscape =
      curCont
      | stdext::and_then(ShouldAdvance{
          .predicate = utility::canEscape && !utility::isUnicodeIndicator,
          .steps = 1,
      });
  if (curEscape.has_value()) {
    return curEscape
         | stdext::and_then(DispatchToConsumer{
             .consumer = consumer,
             .allowedConsumerType = ConsumerType::String});
  }
  return curCont
       | stdext::and_then(ShouldAdvance{
           .predicate = utility::isUnicodeIndicator,
           .errorInfo =
               {.message = "Backslashes should be followed by one of the"
                           "escapable characters: b,f,n,r,t,u,\\,/"},
       })
       | stdext::and_then(ShouldAdvance{
           .predicate = utility::isHexadecimal,
           .steps = 4,
           .errorInfo =
               {.message = "\\u should be followed by 4 hexadecimal digits "
                           "for proper unicode escaping",
                .type = Token::Type::String}})
       | stdext::and_then(DispatchToConsumer{
           .consumer = consumer, .allowedConsumerType = ConsumerType::String});
}

constexpr CursorOrError consumeNumber(
    JsonCursor cursor, Consumer auto& consumer) {
  using namespace details::_parser;
  return CursorOrError(cursor) | stdext::and_then(&shouldSkipIntegerPart)
       | stdext::and_then(&tryAdvanceFractionalPart)
       | stdext::and_then(&tryAdvanceExponentialPart)
       | stdext::and_then(
             Consume{.consumer = consumer, .type = Token::Type::Number});
}

constexpr CursorOrError consumeBoolean(
    JsonCursor cursor, Consumer auto& consumer) {
  using namespace details::_parser;
  return advanceBoolean(cursor)
       | stdext::and_then(
             Consume{.consumer = consumer, .type = Token::Type::Boolean});
}

constexpr CursorOrError consumeNull(
    JsonCursor cursor, Consumer auto& consumer) {
  using namespace details::_parser;
  return advanceNull(cursor)
       | stdext::and_then(
             Consume{.consumer = consumer, .type = Token::Type::Null});
}

}  // namespace injectx::json::parser