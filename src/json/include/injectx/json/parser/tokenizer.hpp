#pragma once

#include <injectx/json/parser/stringparser.hpp>
#include <injectx/json/parser/token.hpp>
#include <injectx/json/parser/utility.hpp>
#include <injectx/stdext/expected.hpp>
#include <injectx/stdext/static_string.hpp>

namespace injectx::json::parser {

struct ParseError {
  std::string_view message_;
  std::size_t parse_begin_;
  std::size_t parse_end_;
  Token::Type token_type_;
};

namespace details {

class JsonReader {
  std::string_view json_;
  std::size_t current_position_ = 0;
  std::size_t marked_position_ = 0;

 public:
  constexpr JsonReader(std::string_view json)
      : json_(json) {
  }

  inline constexpr char peek() const noexcept {
    return json_[current_position_];
  }

  inline constexpr bool canRead() const noexcept {
    return current_position_ < json_.size();
  }

  inline constexpr JsonReader& markCurrent() noexcept {
    marked_position_ = current_position_;
    return *this;
  }

  inline constexpr JsonReader& advance(std::size_t steps = 1) noexcept {
    current_position_ += std::min(steps, json_.size() - current_position_);
    return *this;
  }

  template<typename Predicate>
  inline constexpr JsonReader& advanceIfOnce(Predicate predicate) noexcept {
    if (canRead() && predicate(peek())) {
      advance();
    }
    return *this;
  }

  template<typename Predicate>
  inline constexpr JsonReader& advanceWhile(Predicate predicate) noexcept {
    while (canRead() && predicate(peek())) {
      advance();
    }
    return *this;
  }

  template<typename Predicate>
  inline constexpr JsonReader& shouldAdvanceOnce(Predicate predicate) noexcept {
    if (canRead() && predicate(peek())) {
      advance();
    } else {
      current_position_ = json_.size();
    }
    return *this;
  }

  template<typename Predicate>
  inline constexpr JsonReader& shouldAdvance(
      Predicate predicate, std::size_t steps = 1) noexcept {
    if (steps == 0) {
      return *this;
    }
    while (canRead() && steps > 0) {
      shouldAdvanceOnce(predicate);
    }
    return *this;
  }

  inline constexpr JsonReader& advanceToEnd() noexcept {
    current_position_ = json_.size();
    return *this;
  }

  inline constexpr std::size_t mark() const noexcept {
    return marked_position_;
  }

  inline constexpr std::size_t position() const noexcept {
    return current_position_;
  }

  inline constexpr std::string_view fromMarked(
      std::size_t mark_offset = 0,
      std::size_t current_offset = 0) const noexcept {
    if (current_position_ - marked_position_ + current_offset
        < marked_position_ + mark_offset) {
      return "";
    }
    return json_.substr(
        marked_position_ + mark_offset,
        current_position_ - marked_position_ - current_offset);
  }

  inline constexpr bool isNext(std::string_view str) const noexcept {
    return json_.substr(current_position_, str.size()) == str;
  }

  inline constexpr operator char() const noexcept {
    return peek();
  }

  inline constexpr operator std::string_view() const noexcept {
    return fromMarked();
  }

  inline constexpr operator bool() const noexcept {
    return canRead();
  }
};

enum class ConsumeAction {
  CountTokens,
  ExtractTokens,
};

template<typename T>
concept Consumer = requires(T t, Token token) {
  {
    t.operator()(token)
  } -> std::same_as<stdext::expected<void, std::string_view>>;
};

using JsonStream = stdext::expected<JsonReader, ParseError>;

constexpr bool isTokenStart(JsonReader stream, Token::Type type) noexcept {
  char c = stream.peek();
  switch (type) {
    case Token::Type::ObjectBegin:
      return utility::isLeftBrace(c);
    case Token::Type::ObjectEnd:
      return utility::isRightBrace(c);
    case Token::Type::ArrayBegin:
      return utility::isLeftBracket(c);
    case Token::Type::ArrayEnd:
      return utility::isRightBracket(c);
    case Token::Type::String:
      return utility::isQuote(c);
    case Token::Type::Number:
      return utility::canNumberStartWith(c);
    case Token::Type::Boolean:
      return c == utility::canBooleanStartWith(c);
    case Token::Type::Null:
      return c == utility::canNullStartWith(c);
    default:
      return false;
  }
}

// declatations needed since consumeValue is used in consumeArrayElements and
// consumeKeyValuePairs
//  it is neater to declare all consume* functions together
constexpr JsonStream consumeDocument(JsonReader, Consumer auto&) noexcept;

constexpr JsonStream consumeObject(JsonReader, Consumer auto&) noexcept;

constexpr JsonStream consumeKeyValuePairs(JsonReader, Consumer auto&) noexcept;

constexpr JsonStream consumeArray(JsonReader, Consumer auto&) noexcept;

constexpr JsonStream consumeArrayElements(JsonReader, Consumer auto&) noexcept;

constexpr JsonStream consumeValue(JsonReader, Consumer auto&) noexcept;

constexpr JsonStream consumeString(JsonReader, Consumer auto&) noexcept;

constexpr JsonStream consumeNumber(JsonReader, Consumer auto&) noexcept;

constexpr JsonStream consumeBoolean(JsonReader, Consumer auto&) noexcept;

constexpr JsonStream consumeNull(JsonReader, Consumer auto&) noexcept;

constexpr JsonStream consumeDocument(
    JsonReader stream, Consumer auto& consumer) noexcept {
  stream.advanceWhile(utility::isSpace);
  if (!stream) {
    return stdext::unexpected{ParseError{
        "Empty document", stream.mark(), stream.position(),
        Token::Type::Invalid}};
  }
  stream.markCurrent();
  return JsonStream{stream}
       | stdext::and_then([&consumer](auto reader) -> JsonStream {
           if (isTokenStart(reader, Token::Type::ObjectBegin)) {
             return consumeObject(reader, consumer);
           } else if (isTokenStart(reader, Token::Type::ArrayBegin)) {
             return consumeArray(reader, consumer);
           } else {
             return stdext::unexpected{ParseError{
                 "Document must start with an object or an array",
                 reader.mark(), reader.position(), Token::Type::Invalid}};
           }
         })
       | stdext::and_then([](auto reader) -> JsonStream {
           reader.advanceWhile(utility::isSpace);
           if (reader) {
             reader.markCurrent();
             return stdext::unexpected{ParseError{
                 "Json should only contain one object or one array",
                 reader.mark(), reader.position(), Token::Type::Invalid}};
           }
           return reader;
         });
}

constexpr JsonStream consumeObject(
    JsonReader stream, Consumer auto& consumer) noexcept {
  stream.markCurrent();
  if (!stream.shouldAdvanceOnce(utility::isLeftBrace)) {
    return stdext::unexpected{ParseError{
        "Object must start with '{'", stream.mark(), stream.position(),
        Token::Type::Invalid}};
  }
  std::ignore =
      consumer(Token{.type_ = Token::Type::ObjectBegin, .string_ = stream});
  stream.advanceWhile(utility::isSpace);
  stream.markCurrent();
  // It should either be the end of the object or the start of a key-value
  // pair
  if (utility::isRightBrace(stream.peek())) {
    std::ignore = consumer(
        Token{.type_ = Token::Type::ObjectEnd, .string_ = stream.advance()});
    return stream;
  }
  if (!stream.shouldAdvanceOnce(utility::isQuote)) {
    return stdext::unexpected{ParseError{
        "Object key must be a string", stream.mark(), stream.position(),
        Token::Type::Invalid}};
  }
  stream.markCurrent();
  return consumeKeyValuePairs(stream, consumer);
}

constexpr JsonStream consumeKeyValuePairs(
    JsonReader stream, Consumer auto& consumer) noexcept {
  stdext::expects(
      utility::isQuote(stream.peek()),
      "Programming error, consumeKeyValuePairs must start with a string");
  while (stream) {
    auto result =
        JsonStream{stream}
        | stdext::and_then([&consumer](auto reader) -> JsonStream {
            return consumeString(reader, consumer);
          })
        | stdext::and_then([&consumer](auto reader) -> JsonStream {
            if (!reader.advanceWhile(utility::isSpace)) {
              return stdext::unexpected{ParseError{
                  "Object key-value pair must end with '}'", reader.mark(),
                  reader.position(), Token::Type::Invalid}};
            }
            if (!reader.shouldAdvanceOnce(utility::isColon)) {
              return stdext::unexpected{ParseError{
                  "Object key-value pair must have a colon ':' and a value "
                  "following "
                  "it",
                  reader.mark(), reader.position(), Token::Type::Invalid}};
            }
            return consumeValue(reader, consumer);
          });
    if (!result.has_value()) {
      return result;
    }
    stream = result.value();
    stream.advanceWhile(utility::isSpace);
    stream.markCurrent();
    if (utility::isRightBrace(stream.peek())) {
      std::ignore = consumer(
          Token{.type_ = Token::Type::ObjectEnd, .string_ = stream.advance()});
      return stream.markCurrent();
    } else if (utility::isComma(stream.peek())) {
      stream.advance();
      stream.advanceWhile(utility::isSpace);
      stream.markCurrent();
    } else {
      return stdext::unexpected{ParseError{
          "Object key-value pair must end with '}'", stream.mark(),
          stream.position(), Token::Type::Invalid}};
    }
  }
  stream.markCurrent();
  return stdext::unexpected{ParseError{
      "Reached to the end while searching for key-value pairs", stream.mark(),
      stream.position(), Token::Type::Invalid}};
}

constexpr JsonStream consumeArray(
    JsonReader stream, Consumer auto& consumer) noexcept {
  stream.markCurrent();
  if (!stream.shouldAdvanceOnce(utility::isLeftBracket)) {
    return stdext::unexpected{ParseError{
        "Array must start with '['", stream.mark(), stream.position(),
        Token::Type::Invalid}};
  }
  std::ignore =
      consumer(Token{.type_ = Token::Type::ArrayBegin, .string_ = stream});
  stream.advanceWhile(utility::isSpace);
  stream.markCurrent();
  if (!stream) {
    return stdext::unexpected{ParseError{
        "Array must end with ']'", stream.mark(), stream.position(),
        Token::Type::Invalid}};
  }
  if (utility::isRightBracket(stream.peek())) {
    std::ignore = consumer(
        Token{.type_ = Token::Type::ArrayEnd, .string_ = stream.advance()});
    return stream.markCurrent();
  }
  return consumeArrayElements(stream, consumer);
}

constexpr JsonStream consumeArrayElements(
    JsonReader stream, Consumer auto& consumer) noexcept {
  while (stream) {
    auto result = JsonStream{stream}
                | stdext::and_then([&consumer](auto reader) -> JsonStream {
                    return consumeValue(reader, consumer);
                  });
    if (!result.has_value()) {
      return result;
    }
    stream = result.value();
    stream.advanceWhile(utility::isSpace);
    stream.markCurrent();
    if (utility::isRightBracket(stream.peek())) {
      std::ignore = consumer(
          Token{.type_ = Token::Type::ArrayEnd, .string_ = stream.advance()});
      return stream.markCurrent();
    } else if (utility::isComma(stream.peek())) {
      stream.advance().advanceWhile(utility::isSpace).markCurrent();
    } else {
      return stdext::unexpected{ParseError{
          "Array elements must be separated by a comma ','", stream.mark(),
          stream.position(), Token::Type::Invalid}};
    }
  }
  stream.markCurrent();
  return stdext::unexpected{ParseError{
      "Reached to the end while searching for array elements", stream.mark(),
      stream.position(), Token::Type::Invalid}};
}

constexpr JsonStream consumeValue(
    JsonReader stream, Consumer auto& consumer) noexcept {
  stream.advanceWhile(utility::isSpace);
  stream.markCurrent();
  if (!stream) {
    return stdext::unexpected{ParseError{
        "Empty value", stream.mark(), stream.position(), Token::Type::Invalid}};
  }
  if (isTokenStart(stream, Token::Type::ObjectBegin)) {
    return consumeObject(stream, consumer);
  } else if (isTokenStart(stream, Token::Type::ArrayBegin)) {
    return consumeArray(stream, consumer);
  } else if (isTokenStart(stream, Token::Type::String)) {
    return consumeString(stream, consumer);
  } else if (isTokenStart(stream, Token::Type::Number)) {
    return consumeNumber(stream, consumer);
  } else if (isTokenStart(stream, Token::Type::Boolean)) {
    return consumeBoolean(stream, consumer);
  } else if (isTokenStart(stream, Token::Type::Null)) {
    return consumeNull(stream, consumer);
  } else {
    return stdext::unexpected{ParseError{
        "Invalid value", stream.mark(), stream.position(),
        Token::Type::Invalid}};
  }
}

constexpr JsonStream consumeString(
    JsonReader stream, Consumer auto& consumer) noexcept {
  stream.markCurrent();
  if (!stream.shouldAdvanceOnce(utility::isQuote)) {
    return stdext::unexpected{ParseError{
        "String must start with '\"'", stream.mark(), stream.position(),
        Token::Type::Invalid}};
  }
  stream.markCurrent();
  while (stream) {
    stream.advanceWhile(utility::isQuote || utility::isBackslash);
    if (!stream) {
      return stdext::unexpected{ParseError{
          "String must end with '\"'", stream.mark(), stream.position(),
          Token::Type::Invalid}};
    }
    if (utility::isQuote(stream.peek())) {
      std::ignore = consumer(Token{
          .type_ = Token::Type::String, .string_ = stream.fromMarked(0, 1)});
      return stream.advance();
    }
    stream.advance();
    if (utility::isUnicodeIndicator(stream.peek())) {
      stream.shouldAdvance(utility::isHexadecimal, 4);
    } else {
      stream.advanceIfOnce(utility::canEscape);
    }
    if (!stream) {
      return stdext::unexpected{ParseError{
          "String must end with '\"'", stream.mark(), stream.position(),
          Token::Type::Invalid}};
    }
  }
  return stdext::unexpected{ParseError{
      "Reached to the end while searching for string", stream.mark(),
      stream.position(), Token::Type::Invalid}};
}

constexpr JsonStream consumeNumber(
    JsonReader stream, Consumer auto& consumer) noexcept {
  stream.markCurrent();
  stream.advanceIfOnce(utility::isMinus);
  if (!stream) {
    return stdext::unexpected{ParseError{
        "Expected a number following '-'", stream.mark(), stream.position(),
        Token::Type::Invalid}};
  }
  // 0 should be followed by a dot(.) or exponent(e, E)
  // in cases where it doesn't start with 0, we will consume all the digits
  // and on 0 case we will only consume zero so that both cases can only be
  // followed by the same characters
  if (utility::isZero(stream.peek())) {
    stream.advance();
  } else if (stream.shouldAdvance(utility::isDigit)) {
    stream.advanceWhile(utility::isDigit);
  } else {
    return stdext::unexpected{ParseError{
        "Expected a number after '-'", stream.mark(), stream.position(),
        Token::Type::Invalid}};
  }
  if (!stream) {
    return stdext::unexpected{ParseError{
        "Reached to end while reading a number", stream.mark(),
        stream.position(), Token::Type::Invalid}};
  }
  // Now the integer part is over, let's see if there is any fractional part
  if (utility::isDot(stream.peek())) {
    stream.shouldAdvance(utility::isDigit);
    if (!stream) {
      stream.markCurrent();
      return stdext::unexpected{ParseError{
          "Expected a number following '.'", stream.mark(), stream.position(),
          Token::Type::Invalid}};
    }
    stream.advanceWhile(utility::isDigit);
    if (!stream) {
      stream.markCurrent();
      return stdext::unexpected{ParseError{
          "Reached to end while reading a number", stream.mark(),
          stream.position(), Token::Type::Invalid}};
    }
  }
  if (utility::isExponent(stream.peek())) {
    stream.shouldAdvance(utility::isDigit);
    if (!stream) {
      stream.markCurrent();
      return stdext::unexpected{ParseError{
          "Expected a number following '.'", stream.mark(), stream.position(),
          Token::Type::Invalid}};
    }
    stream.advanceWhile(utility::isDigit);
    if (!stream) {
      stream.markCurrent();
      return stdext::unexpected{ParseError{
          "Reached to end while reading a number", stream.mark(),
          stream.position(), Token::Type::Invalid}};
    }
  }
  std::ignore =
      consumer(Token{.type_ = Token::Type::Number, .string_ = stream});
  return stream;
}

constexpr JsonStream consumeBoolean(
    JsonReader stream, Consumer auto& consumer) noexcept {
  stream.markCurrent();
  if (stream.isNext("true")) {
    stream.advance(4);
    std::ignore =
        consumer(Token{.type_ = Token::Type::Boolean, .string_ = stream});
  } else if (stream.isNext("false")) {
    stream.advance(5);
    std::ignore =
        consumer(Token{.type_ = Token::Type::Boolean, .string_ = stream});
  } else {
    return stdext::unexpected{ParseError{
        "Programming error, code shouldn't be visiting here", stream.mark(),
        stream.position(), Token::Type::Invalid}};
  }
  return stream;
}

constexpr JsonStream consumeNull(
    JsonReader stream, Consumer auto& consumer) noexcept {
  stream.markCurrent();
  if (stream.isNext("null")) {
    stream.advance(4);
    std::ignore =
        consumer(Token{.type_ = Token::Type::Null, .string_ = stream});
  } else {
    return stdext::unexpected{ParseError{
        "Programming error, code shouldn't be visiting here", stream.mark(),
        stream.position(), Token::Type::Invalid}};
  }
  return stream;
}

template<stdext::static_string jsonString>
constexpr stdext::expected<std::size_t, std::string_view>
tokenCount() noexcept {
  struct TokenCounter {
    std::size_t count_ = 0;

    stdext::expected<void, std::string_view> operator()(Token) {
      ++count_;
      return {};
    }
  };

  constexpr std::string_view str(jsonString.data(), jsonString.size());
  TokenCounter counter;
  if (auto result = consumeDocument(str, counter)) {
    return counter.count_;
  } else {
    return stdext::unexpected(result.error());
  }
}

template<stdext::static_string jsonString, std::size_t TokenCount>
constexpr stdext::expected<std::array<Token, TokenCount>, std::string_view>
getRawTokens() noexcept {
  struct Tokenizer {
    std::size_t counter_ = 0;
    std::array<Token, TokenCount> tokens_;

    stdext::expected<void, std::string_view> operator()(Token token) {
      if (counter_ < TokenCount) {
        tokens_[counter_++] = token;
        return {};
      }
      return stdext::unexpected(
          "There are more tokens than the indicated count");
    }
  };

  Tokenizer tokenizer;
  constexpr std::string_view str(jsonString.data(), jsonString.size());
  if (auto result = consumeDocument(str, tokenizer)) {
    return tokenizer.tokens_;
  } else {
    return stdext::unexpected(result.error());
  }
}

template<std::size_t Size>
constexpr std::array<bool, Size> transformNeeded(
    const std::array<Token, Size>& tokens) noexcept {
  std::array<bool, Size> ret;
  for (std::size_t i = 0; i < Size; ++i) {
    if (tokens[i].type_ == Token::Type::String
        && utility::hasBackslash(tokens[i].string_)) {
      ret[i] = true;
    } else {
      ret[i] = false;
    }
  }
  return ret;
}

template<const Token& token, bool needed>
constexpr Token transformToken() {
  if constexpr (!needed) {
    return token;
  }
  return Token{
      .string_ = parseString<STDEXT_AS_STATIC_STRING(token.string_)>(),
      .type_ = token.type_};
}

template<
    std::size_t Size,
    const std::array<Token, Size>& rawTokens,
    const std::array<bool, Size>& shouldTransform,
    std::size_t... Indices>
constexpr auto parsedTokens(std::index_sequence<Indices...>) {
  return std::array<Token, Size>{
      transformToken<rawTokens[Indices], shouldTransform[Indices]>()...};
}

template<stdext::static_string json>
constexpr auto processedTokens() {
  if constexpr (auto count = tokenCount<json>()) {
    constexpr auto rawTokens = getRawTokens<json, count>();
    return parsedTokens<>(std::make_index_sequence<count>());
  } else {
    return stdext::expected<void, ParseError>(
        stdext::unexpected(count.error()));
  }
}

}  // namespace details

}  // namespace injectx::json::parser
