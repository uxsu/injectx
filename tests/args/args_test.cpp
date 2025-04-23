#include "injectx/args/args.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace injectx::args::tests {

TEST_CASE("args::arg") {
  using namespace args;
  constexpr auto add = arg + 5;
  REQUIRE(add(10) == 15);

  constexpr auto subtract = arg - 'a';
  REQUIRE(subtract('b') == 1);

  constexpr auto multiply = arg * arg;
  REQUIRE(multiply(5.0) == 25.0);

  constexpr auto divide = arg / 5;
  REQUIRE(divide(25) == 5);
  REQUIRE(divide(1.0) == 0.2);

  constexpr auto modulo = arg % 5;
  REQUIRE(modulo(4) == 4);
  REQUIRE(modulo(7) == 2);

  constexpr auto negate = -arg;
  REQUIRE(negate(5) == -5);

  constexpr auto equal = arg == 5;
  REQUIRE_FALSE(equal(4));
  REQUIRE(equal(5));
  REQUIRE_FALSE(equal(6));

  constexpr auto notEqual = arg != 5;
  REQUIRE(notEqual(4));
  REQUIRE_FALSE(notEqual(5));
  REQUIRE(notEqual(6));

  constexpr auto greater = arg > 5;
  REQUIRE_FALSE(greater(4));
  REQUIRE_FALSE(greater(5));
  REQUIRE(greater(6));

  constexpr auto greaterEqual = arg >= 5;
  REQUIRE_FALSE(greaterEqual(4));
  REQUIRE(greaterEqual(5));
  REQUIRE(greaterEqual(6));

  constexpr auto less = arg < 5;
  REQUIRE(less(4));
  REQUIRE_FALSE(less(5));
  REQUIRE_FALSE(less(6));

  constexpr auto lessEqual = arg <= 5;
  REQUIRE(lessEqual(4));
  REQUIRE(lessEqual(5));
  REQUIRE_FALSE(lessEqual(6));

  constexpr auto compareThreeWay = arg <=> 5;
  REQUIRE(std::is_eq(compareThreeWay(5)));
  REQUIRE(std::is_lt(compareThreeWay(3)));
  REQUIRE(std::is_gt(compareThreeWay(7)));

  constexpr auto bitwiseAnd = arg & 5;
  REQUIRE(bitwiseAnd(0b1010) == (0b1010 & 5));

  constexpr auto bitwiseOr = arg | 5;
  REQUIRE(bitwiseOr(0b1010) == (0b1010 | 5));

  constexpr auto bitwiseXor = arg ^ 5;
  REQUIRE(bitwiseXor(0b1010) == (0b1010 ^ 5));

  constexpr auto bitwiseNot = ~arg;
  REQUIRE(bitwiseNot(0b1010) == ~0b1010);

  constexpr auto logicalAnd = arg && true;
  REQUIRE(logicalAnd(true));
  REQUIRE_FALSE(logicalAnd(false));

  constexpr auto logicalOr = arg || false;
  REQUIRE(logicalOr(true));
  REQUIRE_FALSE(logicalOr(false));

  constexpr auto logicalNot = !arg;
  REQUIRE_FALSE(logicalNot(true));
  REQUIRE(logicalNot(false));
  struct Test {
    std::string string;
  };
  constexpr auto projection = args::arg(&Test::string);
  Test t{"abc"};
  auto result = projection + projection;
  REQUIRE(result(t) == std::string("abcabc"));

}

TEST_CASE("xy-z") {
  using namespace args;
  constexpr auto add = xy::x + xy::y;
  REQUIRE(add(5, 10) == 15);

  constexpr auto subtract = xy::x - xy::y;
  REQUIRE(subtract(10, 5) == 5);

  constexpr auto multiply = xy::x * xy::y;
  REQUIRE(multiply(5, 10) == 50);

  constexpr auto divide = xy::x / xy::y;
  REQUIRE(divide(10, 5) == 2);

  constexpr auto modulo = xy::x % xy::y;
  REQUIRE(modulo(10, 5) == 0);

  constexpr auto negate = -xy::x;
  REQUIRE(negate(5) == -5);

  constexpr auto equal = xy::x == xy::y;
  REQUIRE_FALSE(equal(5, 10));
  REQUIRE(equal(5, 5));
  REQUIRE_FALSE(equal(0, 5));

  constexpr auto notEqual = xy::x != xy::y;
  REQUIRE(notEqual(5, 10));
  REQUIRE_FALSE(notEqual(5, 5));
  REQUIRE(notEqual(0, 5));

  constexpr auto greater = xy::x > xy::y;
  REQUIRE_FALSE(greater(5, 10));
  REQUIRE_FALSE(greater(5, 5));
  REQUIRE(greater(10, 5));

  constexpr auto greaterEqual = xy::x >= xy::y;
  REQUIRE_FALSE(greaterEqual(5, 10));
  REQUIRE(greaterEqual(5, 5));
  REQUIRE(greaterEqual(10, 5));

  constexpr auto less = xy::x < xy::y;
  REQUIRE(less(5, 10));
  REQUIRE_FALSE(less(5, 5));
  REQUIRE_FALSE(less(10, 5));

  constexpr auto lessEqual = xy::x <= xy::y;
  REQUIRE(lessEqual(5, 10));
  REQUIRE(lessEqual(5, 5));
  REQUIRE_FALSE(lessEqual(10, 5));

  constexpr auto compareThreeWay = xy::x <=> xy::y;
  REQUIRE(std::is_eq(compareThreeWay(5, 5)));
  REQUIRE(std::is_lt(compareThreeWay(3, 5)));
  REQUIRE(std::is_gt(compareThreeWay(7, 5)));

  constexpr auto bitwiseAnd = xy::x & xy::y;
  REQUIRE(bitwiseAnd(0b1010, 0b1100) == (0b1010 & 0b1100));

  constexpr auto bitwiseOr = xy::x | xy::y;
  REQUIRE(bitwiseOr(0b1010, 0b1100) == (0b1010 | 0b1100));

  constexpr auto bitwiseXor = xy::x ^ xy::y;
  REQUIRE(bitwiseXor(0b1010, 0b1100) == (0b1010 ^ 0b1100));

  constexpr auto bitwiseNot = ~xy::x;
  REQUIRE(bitwiseNot(0b1010) == ~0b1010);

  constexpr auto logicalAnd = xy::x && xy::y;
  REQUIRE_FALSE(logicalAnd(true, false));

  constexpr auto logicalOr = xy::x || xy::y;
  REQUIRE(logicalOr(true, false));

  constexpr auto logicalNot = !xy::x;
  REQUIRE_FALSE(logicalNot(true));

  constexpr auto logicalNot_x = !xy::x;
  REQUIRE(logicalNot_x(false));

  constexpr auto logicalNot_x_and_y = !xy::x && !xy::y;
  REQUIRE(logicalNot_x_and_y(false, false));

  constexpr auto logicalNot_x_or_y = !xy::x || !xy::y;
  REQUIRE_FALSE(logicalNot_x_or_y(true, true));

  constexpr auto sum_of_squares = xy::x * xy::x + xy::y * xy::y;
  REQUIRE(sum_of_squares(3, 4) == 25);

  constexpr auto sum_of_squares_triplets =
      xy::x * xy::x + xy::y * xy::y + xyz::z * xyz::z;
  REQUIRE(sum_of_squares_triplets(3, 4, 12) == 169);
}

}  // namespace injectx::args::tests
