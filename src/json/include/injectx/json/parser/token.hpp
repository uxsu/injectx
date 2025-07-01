#pragma once

#include <fmt/format.h>

#include <string_view>

namespace injectx::json::parser {

struct Token {
  std::string_view string_;
  enum class Type {
    Invalid,
    DocumentBegin,
    DocumentEnd,
    ObjectBegin,
    ObjectEnd,
    ArrayBegin,
    ArrayEnd,
    String,
    Number,
    Boolean,
    Null,
  } type_ = Type::Invalid;
};

}  // namespace injectx::json::parser

// Useful for debugging
template<>
struct fmt::formatter<injectx::json::parser::Token::Type> {
  constexpr auto parse(auto& ctx) {
    return ctx.begin();
  }

  template<typename FormatContext>
  constexpr auto format(
      injectx::json::parser::Token::Type value, FormatContext& ctx) const {
    switch (value) {
      case injectx::json::parser::Token::Type::Invalid:
        return format_to(ctx.out(), "Token::Type::Invalid");
      case injectx::json::parser::Token::Type::DocumentBegin:
        return format_to(ctx.out(), "Token::Type::DocumentBegin");
      case injectx::json::parser::Token::Type::DocumentEnd:
        return format_to(ctx.out(), "Token::Type::DocumentEnd");
      case injectx::json::parser::Token::Type::ObjectBegin:
        return format_to(ctx.out(), "Token::Type::ObjectBegin");
      case injectx::json::parser::Token::Type::ObjectEnd:
        return format_to(ctx.out(), "Token::Type::ObjectEnd");
      case injectx::json::parser::Token::Type::ArrayBegin:
        return format_to(ctx.out(), "Token::Type::ArrayBegin");
      case injectx::json::parser::Token::Type::ArrayEnd:
        return format_to(ctx.out(), "Token::Type::ArrayEnd");
      case injectx::json::parser::Token::Type::String:
        return format_to(ctx.out(), "Token::Type::String");
      case injectx::json::parser::Token::Type::Number:
        return format_to(ctx.out(), "Token::Type::Number");
      case injectx::json::parser::Token::Type::Boolean:
        return format_to(ctx.out(), "Token::Type::Boolean");
      case injectx::json::parser::Token::Type::Null:
        return format_to(ctx.out(), "Token::Type::Null");
    }
  }
};
