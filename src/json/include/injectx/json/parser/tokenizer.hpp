#pragma once

#include <injectx/json/parser/token.hpp>

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

  inline constexpr char peek() const {
    return json_[current_position_];
  }

  inline constexpr bool canRead() const {
    return current_position_ < json_.size();
  }

  inline constexpr JsonReader& mark() {
    marked_position_ = current_position_;
    return *this;
  }

  inline constexpr JsonReader& advance(std::size_t steps = 1) {
    current_position_ += std::min(steps, json_.size() - current_position_);
    return *this;
  }

  template<typename Predicate>
  inline constexpr JsonReader& advanceIfOnce(Predicate predicate) {
    if (canRead() && predicate(peek())) {
      advance();
    }
    return *this;
  }

  template<typename Predicate>
  inline constexpr JsonReader& advanceWhile(Predicate predicate) {
    while (canRead() && predicate(peek())) {
      advance();
    }
    return *this;
  }


};

};  // namespace details

}  // namespace injectx::json::parser