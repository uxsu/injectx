#include "injectx/math/bigint.hpp"

#include <limits>

#include "catch2/catch_test_macros.hpp"

namespace injectx::math::tests {

inline constexpr std::uint64_t base = 256;

constexpr std::uint8_t createValue(std::uint8_t d0) {
  return d0;
}

constexpr std::uint16_t createValue(std::uint8_t d0, std::uint8_t d1) {
  return static_cast<std::uint16_t>(createValue(d0))
       + static_cast<std::uint16_t>(createValue(d1))
             * (static_cast<std::uint16_t>(
                    std::numeric_limits<std::uint8_t>::max())
                + static_cast<std::uint16_t>(1));
}

constexpr std::uint32_t createValue(
    std::uint8_t d0, std::uint8_t d1, std::uint8_t d2, std::uint8_t d3) {
  return static_cast<std::uint32_t>(createValue(d0, d1))
       + static_cast<std::uint32_t>(createValue(d2, d3))
             * (static_cast<std::uint32_t>(
                    std::numeric_limits<std::uint16_t>::max())
                + static_cast<std::uint32_t>(1));
}

constexpr std::uint64_t createValue(
    std::uint8_t d0,
    std::uint8_t d1,
    std::uint8_t d2,
    std::uint8_t d3,
    std::uint8_t d4,
    std::uint8_t d5,
    std::uint8_t d6,
    std::uint8_t d7) {
  return static_cast<std::uint64_t>(createValue(d0, d1, d2, d3))
       + static_cast<std::uint64_t>(createValue(d4, d5, d6, d7))
             * (static_cast<std::uint64_t>(
                    std::numeric_limits<std::uint32_t>::max())
                + static_cast<std::uint64_t>(1));
}

using namespace math_literals;

TEST_CASE("UnsignedBigInt-constructor") {
  SECTION("uint8") {
    constexpr std::uint8_t d0 = 57;
    constexpr auto value = createValue(d0);
    UnsignedBigInt ubi{value};
    REQUIRE(ubi[0] == d0);
    REQUIRE(ubi[1] == 0);
  }

  SECTION("uint16") {
    constexpr std::uint8_t d0 = 157;
    constexpr std::uint8_t d1 = 7;
    constexpr auto value = createValue(d0, d1);
    UnsignedBigInt ubi{value};
    REQUIRE(ubi[0] == d0);
    REQUIRE(ubi[1] == d1);
    REQUIRE(ubi[2] == 0);
  }
  SECTION("uint32") {
    constexpr std::uint8_t d0 = 15;
    constexpr std::uint8_t d1 = 78;
    constexpr std::uint8_t d2 = 89;
    constexpr std::uint8_t d3 = 90;
    constexpr auto value = createValue(d0, d1, d2, d3);
    UnsignedBigInt ubi{value};
    REQUIRE(ubi[0] == d0);
    REQUIRE(ubi[1] == d1);
    REQUIRE(ubi[2] == d2);
    REQUIRE(ubi[3] == d3);
    REQUIRE(ubi[4] == 0);
  }
  SECTION("uint64") {
    constexpr std::uint8_t d0 = 15;
    constexpr std::uint8_t d1 = 78;
    constexpr std::uint8_t d2 = 89;
    constexpr std::uint8_t d3 = 90;
    constexpr std::uint8_t d4 = 0;
    constexpr std::uint8_t d5 = 9;
    constexpr std::uint8_t d6 = 2;
    constexpr std::uint8_t d7 = 255;
    constexpr auto value = createValue(d0, d1, d2, d3, d4, d5, d6, d7);
    UnsignedBigInt ubi{value};
    REQUIRE(ubi[0] == d0);
    REQUIRE(ubi[1] == d1);
    REQUIRE(ubi[2] == d2);
    REQUIRE(ubi[3] == d3);
    REQUIRE(ubi[4] == d4);
    REQUIRE(ubi[5] == d5);
    REQUIRE(ubi[6] == d6);
    REQUIRE(ubi[7] == d7);
    REQUIRE(ubi[8] == 0);
  }
  SECTION("vector<uint8>") {
    STATIC_REQUIRE(
        UnsignedBigInt{{1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0}}
        == UnsignedBigInt{{1, 2, 3, 4, 5, 6, 7, 8}});
    STATIC_REQUIRE(
        UnsignedBigInt{{1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0}}[0] == 1);
    STATIC_REQUIRE(
        UnsignedBigInt{{1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0}}[1] == 2);
    STATIC_REQUIRE(
        UnsignedBigInt{{1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0}}[2] == 3);
    STATIC_REQUIRE(
        UnsignedBigInt{{1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0}}[3] == 4);
    STATIC_REQUIRE(
        UnsignedBigInt{{1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0}}[4] == 5);
    STATIC_REQUIRE(
        UnsignedBigInt{{1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0}}[5] == 6);
    STATIC_REQUIRE(
        UnsignedBigInt{{1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0}}[6] == 7);
    STATIC_REQUIRE(
        UnsignedBigInt{{1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0}}[7] == 8);
    STATIC_REQUIRE(
        UnsignedBigInt{{1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0}}[8] == 0);
    STATIC_REQUIRE(UnsignedBigInt{{1, 2, 3, 4, 5, 6, 7, 8}}[8] == 0);
  }
}

TEST_CASE("UnsignedBigInt-comparison") {
  STATIC_REQUIRE(80_ubi == 80_ubi);
  STATIC_REQUIRE(80_ubi < 81_ubi);
  STATIC_REQUIRE(80_ubi <= 81_ubi);
  STATIC_REQUIRE(81_ubi > 80_ubi);
  STATIC_REQUIRE(81_ubi >= 80_ubi);
  STATIC_REQUIRE(81_ubi != 80_ubi);
}

TEST_CASE("UnsignedBigInt-sum") {
  STATIC_REQUIRE(80_ubi + 80_ubi == 160_ubi);
  STATIC_REQUIRE(1000000000080_ubi + 1000000000080_ubi == 2000000000160_ubi);
  STATIC_REQUIRE(0_ubi + 4930592069_ubi == 4930592069_ubi);
  STATIC_REQUIRE(
      53460_ubi + 4930592069_ubi == UnsignedBigInt{53460u + 4930592069u});
  STATIC_REQUIRE(256_ubi + 256_ubi + 256_ubi + 256_ubi == 512_ubi + 512_ubi);
  STATIC_REQUIRE(
      65536_ubi + 65536_ubi + 65536_ubi + 65536_ubi == 131072_ubi + 131072_ubi);
  STATIC_REQUIRE(
      16777216_ubi + 16777216_ubi + 65536_ubi + 65536_ubi
      == 33554432_ubi + 131072_ubi);
}

TEST_CASE("UnsignedBigInt-shift") {
  STATIC_REQUIRE(80_ubi >> 1 == 0_ubi);
  STATIC_REQUIRE(80_ubi << 1 == UnsignedBigInt{256u * 80u});
  STATIC_REQUIRE(
      UnsignedBigInt{{1, 2, 3, 4, 5, 6, 7, 8, 9}} << 5
      == UnsignedBigInt{{0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9}});
  STATIC_REQUIRE(
      UnsignedBigInt{{1, 2, 3, 4, 5, 6, 7, 8, 9}} >> 5
      == UnsignedBigInt{{6, 7, 8, 9}});
  UnsignedBigInt val{{1, 2, 78, 90}};
  val <<= 20;
  std::size_t i = 0;
  for (; i < 20; ++i) {
    REQUIRE(val[i] == 0);
  }
  REQUIRE(val[i++] == 1);
  REQUIRE(val[i++] == 2);
  REQUIRE(val[i++] == 78);
  REQUIRE(val[i++] == 90);
  val >>= 10;
  i = 0;
  for (; i < 10; ++i) {
    REQUIRE(val[i] == 0);
  }
  REQUIRE(val[i++] == 1);
  REQUIRE(val[i++] == 2);
  REQUIRE(val[i++] == 78);
  REQUIRE(val[i++] == 90);
  val >>= 10;
  REQUIRE(val == UnsignedBigInt{{1, 2, 78, 90}});
  REQUIRE((val >>= 4) == UnsignedBigInt{0u});
}

}  // namespace injectx::math::tests
