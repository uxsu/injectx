#pragma once
#include "injectx/args/args.hpp"
#include "injectx/stdext/expects.hpp"

#include <algorithm>
#include <compare>
#include <concepts>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <span>
#include <vector>

namespace injectx::math {

namespace details::_math {
constexpr std::uint8_t sum(
    std::uint8_t lhs, std::uint8_t rhs, std::uint8_t& carry) {
  std::uint16_t result = static_cast<std::uint16_t>(lhs)
                       + static_cast<std::uint16_t>(rhs)
                       + static_cast<std::uint16_t>(carry);
  carry = static_cast<std::uint8_t>(result >> 8);
  return static_cast<std::uint8_t>(result);
}

constexpr auto partitionInto8Bits(std::unsigned_integral auto value) {
  if (value == 0) {
    return std::vector<std::uint8_t>{0};
  }
  std::vector<std::uint8_t> partition;
  constexpr auto size = sizeof(value);
  for (std::size_t i = 0; i < size && value != 0; ++i) {
    partition.push_back(static_cast<std::uint8_t>(value));
    value >>= 8;
  }
  return partition;
}
}  // namespace details::_math

struct UnsignedBigInt {
  std::vector<std::uint8_t> buffer_;

  constexpr std::uint8_t operator[](std::size_t index) const {
    if (index < buffer_.size()) {
      return buffer_[index];
    }
    return 0;
  }

  constexpr UnsignedBigInt() = default;

  constexpr UnsignedBigInt(std::unsigned_integral auto val)
      : buffer_(details::_math::partitionInto8Bits(val)) {
  }

  constexpr UnsignedBigInt(const std::vector<std::uint8_t>& buffer)
      : buffer_(
            buffer | std::views::reverse
            | std::views::drop_while(args::arg == 0) | std::views::reverse
            | std::ranges::to<std::vector>()) {
  }

  constexpr UnsignedBigInt(const UnsignedBigInt&) = default;
  constexpr UnsignedBigInt(UnsignedBigInt&&) = default;
  constexpr UnsignedBigInt& operator=(const UnsignedBigInt&) = default;
  constexpr UnsignedBigInt& operator=(UnsignedBigInt&&) = default;
  constexpr ~UnsignedBigInt() = default;

  constexpr const auto data() const noexcept {
    return buffer_.data();
  };

  constexpr auto trailingZeros() const noexcept {
    return static_cast<std::size_t>(std::ranges::distance(
        buffer_ | std::views::take_while(args::arg == 0)));
  }

  friend constexpr UnsignedBigInt operator>>(
      const UnsignedBigInt& data, std::size_t index) noexcept;
  friend constexpr UnsignedBigInt operator<<(
      const UnsignedBigInt& data, std::size_t index) noexcept;
  friend constexpr UnsignedBigInt& operator>>=(
      UnsignedBigInt& data, std::size_t index) noexcept;
  friend constexpr UnsignedBigInt& operator<<=(
      UnsignedBigInt& data, std::size_t index) noexcept;
  friend constexpr UnsignedBigInt operator+(
      const UnsignedBigInt& lhs, const UnsignedBigInt& rhs) noexcept;
  friend constexpr UnsignedBigInt operator*(
      const UnsignedBigInt& lhs, const UnsignedBigInt& rhs) noexcept;
  constexpr std::strong_ordering operator<=>(
      const UnsignedBigInt& other) const noexcept;
  constexpr bool operator==(const UnsignedBigInt& other) const noexcept;
};

constexpr UnsignedBigInt operator>>(
    const UnsignedBigInt& data, std::size_t index) noexcept {
  if (index >= data.buffer_.size()) {
    return UnsignedBigInt{0u};
  }
  UnsignedBigInt result;
  std::ranges::copy(
      data.buffer_ | std::views::drop(index),
      std::back_inserter(result.buffer_));
  return result;
}

constexpr UnsignedBigInt operator<<(
    const UnsignedBigInt& data, std::size_t index) noexcept {
  UnsignedBigInt result;
  result.buffer_.resize(index);
  std::ranges::copy(data.buffer_, std::back_inserter(result.buffer_));
  return result;
}

constexpr UnsignedBigInt& operator>>=(
    UnsignedBigInt& data, std::size_t index) noexcept {
  auto prevSize = data.buffer_.size();
  if (index >= prevSize) {
    return data = UnsignedBigInt{0u};
  }
  UnsignedBigInt result;
  std::ranges::copy(
      data.buffer_ | std::views::drop(index), std::ranges::begin(data.buffer_));
  data.buffer_.resize(prevSize - index);
  return data;
}

constexpr UnsignedBigInt& operator<<=(
    UnsignedBigInt& data, std::size_t index) noexcept {
  auto prevSize = data.buffer_.size();
  data.buffer_.resize(prevSize + index);
  std::ranges::copy_backward(
      data.buffer_ | std::views::take(prevSize),
      std::ranges::end(data.buffer_));
  std::ranges::fill_n(std::ranges::begin(data.buffer_), index, 0u);
  return data;
}

constexpr UnsignedBigInt operator+(
    const UnsignedBigInt& lhs, const UnsignedBigInt& rhs) noexcept {
  UnsignedBigInt result;
  auto max = std::max(lhs.buffer_.size(), rhs.buffer_.size());
  std::uint8_t carry = 0;
  for (std::size_t i = 0; i < max; ++i) {
    result.buffer_.push_back(details::_math::sum(lhs[i], rhs[i], carry));
  }
  if (carry > 0) {
    result.buffer_.push_back(carry);
  }
  return result;
}

constexpr std::strong_ordering UnsignedBigInt::operator<=>(
    const UnsignedBigInt& other) const noexcept {
  if (buffer_.size() != other.buffer_.size()) {
    return buffer_.size() <=> other.buffer_.size();
  }
  for (std::size_t i = buffer_.size(); i-- > 0;) {
    if (buffer_[i] != other.buffer_[i]) {
      return buffer_[i] <=> other.buffer_[i];
    }
  }
  return std::strong_ordering::equal;
}

constexpr bool UnsignedBigInt::operator==(
    const UnsignedBigInt& other) const noexcept {
  return buffer_ == other.buffer_;
}
}  // namespace injectx::math

namespace injectx::math_literals {
constexpr auto operator""_ubi(unsigned long long value) {
  return math::UnsignedBigInt{value};
}
}  // namespace injectx::math_literals
