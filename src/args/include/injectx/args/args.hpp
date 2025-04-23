#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace injectx::args {
namespace details {

template<typename Type>
concept MemberPointer = std::is_member_pointer_v<std::remove_reference_t<Type>>;

// template <typename Type>
// concept MemberFunction = std::is_member_function_pointer_v<Type>;

template<typename Tuple, typename Callable, int arg_position>
constexpr auto call(Tuple t, Callable callable) {
  return callable(std::get<arg_position>(t));
}

template<MemberPointer Callable>
struct MemberPointerCallable {
  Callable callable;
  template<typename T>
  constexpr auto operator()(T t) {
    if constexpr (std::is_pointer_v<std::remove_reference_t<T>>) {
      return t->*(callable);
    } else {
      return t.*(callable);
    }
  }
};

template<MemberPointer Callable>
constexpr auto makeMemberPointerCallable(Callable callable) {
  return MemberPointerCallable<Callable>{callable};
}

// template<typename Tuple, MemberFunction Callable, int arg_position>
// constexpr auto call(Tuple t, Callable callable) {
//   if constexpr
//   (std::is_pointer_v<std::remove_reference_t<std::tuple_element_t<arg_position,
//   Tuple>>>) {
//     return callable(*std::get<arg_position>(t));
//   }
//   else {
//     return callable(std::get<arg_position>(t));
//   }
// }

// Required to have a way to differentiate between arg and other types
struct ArgTag {};

template<typename T>
concept ArgLike = requires {
  typename std::remove_cvref_t<T>::Tag;
  typename std::remove_cvref_t<T>::ContextTag;
  requires std::same_as<typename std::remove_cvref_t<T>::Tag, ArgTag>;
};

template<typename ArgLike1, typename ArgLike2>
concept SharesContextWith = requires {
  requires ArgLike<ArgLike1>;
  requires ArgLike<ArgLike2>;
  requires std::derived_from<
               typename std::remove_cvref_t<ArgLike1>::ContextTag,
               typename std::remove_cvref_t<ArgLike2>::ContextTag>
               || std::derived_from<
                   typename std::remove_cvref_t<ArgLike2>::ContextTag,
                   typename std::remove_cvref_t<ArgLike1>::ContextTag>;
};

template<ArgLike ArgLike1, ArgLike ArgLike2>
struct common_context : std::conditional<
                            std::derived_from<
                                typename ArgLike1::ContextTag,
                                typename ArgLike2::ContextTag>,
                            typename ArgLike1::ContextTag,
                            typename ArgLike2::ContextTag> {};

template<ArgLike ArgLike1, ArgLike ArgLike2>
using common_context_t = typename common_context<ArgLike1, ArgLike2>::type;

template<class Callable, int arg_position, typename Context>
struct Arg {
  using Tag = ArgTag;
  using ContextTag = Context;
  using CallableType = Callable;
  CallableType callable_;

  constexpr Arg(Callable&& callable) : callable_(callable) {
  }

  template<typename... Args>
  constexpr auto operator()(Args&&... args) const {
    if constexpr (arg_position != -1) {
      auto t = std::make_tuple(args...);
      return details::call<decltype(t), CallableType, arg_position>(
          t, callable_);
    } else {
      return callable_(args...);
    }
  }

  template<MemberPointer MemberPointerType>
  constexpr auto operator()(MemberPointerType callable) const {
    return Arg<MemberPointerCallable<MemberPointerType>, arg_position, Context>(std::move(makeMemberPointerCallable(callable)));
  }

  template<typename... Args>
  constexpr auto operator()(Args&... args) const {
    if constexpr (arg_position != -1) {
      auto t = std::make_tuple(args...);
      return details::call<decltype(t), CallableType, arg_position>(
          t, callable_);
    } else {
      return callable_(args...);
    }
  }

  template<typename Operator, ArgLike OtherArgLike>
  constexpr auto thisComposeOther(OtherArgLike f, Operator) const {
    static_assert(
        SharesContextWith<
            OtherArgLike, Arg<CallableType, arg_position, Context>>,
        "The arguments must share the same context, make sure not to mix "
        "arguments from unrelated namespaces.");
    auto composed = [l = *this, r = f](auto&&... args) {
      Operator op{};
      return op(l(args...), r(args...));
    };
    using CommonContext = common_context_t<
        OtherArgLike, Arg<CallableType, arg_position, Context>>;
    return Arg<decltype(composed), -1, CommonContext>(std::move(composed));
  }

  template<typename Value, typename Operator>
  constexpr auto valueComposeThis(Value v, Operator) const {
    auto composed = [callable = *this, v](auto&&... args) {
      Operator op{};
      return op(v, callable(args...));
    };
    return Arg<decltype(composed), -1, Context>(std::move(composed));
  }

  template<typename Value, typename Operator>
  constexpr auto thisComposeValue(Value v, Operator) const {
    auto composed = [callable = *this, v](auto&&... args) {
      Operator op{};
      return op(callable(args...), v);
    };
    return Arg<decltype(composed), -1, Context>(std::move(composed));
  }

  template<typename Operator>
  constexpr auto thisComposeUnary(Operator) const {
    auto composed = [callable = *this](auto&&... args) {
      Operator op{};
      return op(callable(args...));
    };
    return Arg<decltype(composed), -1, Context>(std::move(composed));
  }
};

#define ARGLIKE_UNARY_OPERATOR_OVERLOADING(OPERATOR, FUNCTIONAL_WRAPPER) \
  constexpr auto operator OPERATOR(ArgLike auto&& arglike) {             \
    return arglike.thisComposeUnary(FUNCTIONAL_WRAPPER{});               \
  }

#define ARGLIKE_BINARY_OPERATOR_OVERLOADING(OPERATOR, FUNCTIONAL_WRAPPER) \
  constexpr auto operator OPERATOR(                                       \
      ArgLike auto&& arglike1, ArgLike auto&& arglike2) {                 \
    return arglike1.thisComposeOther(arglike2, FUNCTIONAL_WRAPPER{});     \
  }                                                                       \
                                                                          \
  template<typename T>                                                    \
    requires(!ArgLike<T>)                                                 \
  constexpr auto operator OPERATOR(ArgLike auto&& arglike, T&& value) {   \
    return arglike.thisComposeValue(value, FUNCTIONAL_WRAPPER{});         \
  }                                                                       \
                                                                          \
  template<typename T>                                                    \
    requires(!ArgLike<T>)                                                 \
  constexpr auto operator OPERATOR(T&& value, ArgLike auto&& arglike) {   \
    return arglike.valueComposeThis(value, FUNCTIONAL_WRAPPER{});         \
  }

ARGLIKE_BINARY_OPERATOR_OVERLOADING(+, std::plus)
ARGLIKE_BINARY_OPERATOR_OVERLOADING(-, std::minus)
ARGLIKE_BINARY_OPERATOR_OVERLOADING(*, std::multiplies)
ARGLIKE_BINARY_OPERATOR_OVERLOADING(/, std::divides)
ARGLIKE_BINARY_OPERATOR_OVERLOADING(%, std::modulus)
ARGLIKE_UNARY_OPERATOR_OVERLOADING(-, std::negate)

ARGLIKE_BINARY_OPERATOR_OVERLOADING(==, std::equal_to)
ARGLIKE_BINARY_OPERATOR_OVERLOADING(!=, std::not_equal_to)
ARGLIKE_BINARY_OPERATOR_OVERLOADING(>, std::greater)
ARGLIKE_BINARY_OPERATOR_OVERLOADING(>=, std::greater_equal)
ARGLIKE_BINARY_OPERATOR_OVERLOADING(<, std::less)
ARGLIKE_BINARY_OPERATOR_OVERLOADING(<=, std::less_equal)
ARGLIKE_BINARY_OPERATOR_OVERLOADING(<=>, std::compare_three_way)

ARGLIKE_BINARY_OPERATOR_OVERLOADING(&&, std::logical_and)
ARGLIKE_BINARY_OPERATOR_OVERLOADING(||, std::logical_or)
ARGLIKE_UNARY_OPERATOR_OVERLOADING(!, std::logical_not)

ARGLIKE_BINARY_OPERATOR_OVERLOADING(&, std::bit_and)
ARGLIKE_BINARY_OPERATOR_OVERLOADING(|, std::bit_or)
ARGLIKE_BINARY_OPERATOR_OVERLOADING(^, std::bit_xor)
ARGLIKE_UNARY_OPERATOR_OVERLOADING(~, std::bit_not)

template<int Number, typename Context>
constexpr auto makeArg() {
  return Arg<std::identity, Number, Context>(std::identity{});
}

namespace contexts {
struct NoContextTag {};

struct NumberedContextTag {};

struct xyContextTag {};

struct xyzContextTag : xyContextTag {};

struct abContextTag {};

struct abcContextTag : abContextTag {};

struct ijContextTag {};

struct ijkContextTag : ijContextTag {};

struct uvContextTag {};

}  // namespace contexts
}  // namespace details

constexpr auto arg = details::makeArg<0, details::contexts::NoContextTag>();

namespace numbered {
constexpr auto _1 =
    details::makeArg<0, details::contexts::NumberedContextTag>();
constexpr auto _2 =
    details::makeArg<1, details::contexts::NumberedContextTag>();
constexpr auto _3 =
    details::makeArg<2, details::contexts::NumberedContextTag>();
constexpr auto _4 =
    details::makeArg<3, details::contexts::NumberedContextTag>();
constexpr auto _5 =
    details::makeArg<4, details::contexts::NumberedContextTag>();
}  // namespace numbered

namespace xy {
constexpr auto x = details::makeArg<0, details::contexts::xyContextTag>();
constexpr auto y = details::makeArg<1, details::contexts::xyContextTag>();

}  // namespace xy

namespace xyz {
using namespace xy;
constexpr auto z = details::makeArg<2, details::contexts::xyzContextTag>();
}  // namespace xyz

namespace ab {
constexpr auto a = details::makeArg<0, details::contexts::abContextTag>();
constexpr auto b = details::makeArg<1, details::contexts::abContextTag>();
}  // namespace ab

namespace abc {
using namespace ab;
constexpr auto c = details::makeArg<2, details::contexts::abcContextTag>();
}  // namespace abc

namespace ij {
constexpr auto i = details::makeArg<0, details::contexts::ijContextTag>();
constexpr auto j = details::makeArg<1, details::contexts::ijContextTag>();
}  // namespace ij

namespace ijk {
using namespace ij;
constexpr auto k = details::makeArg<2, details::contexts::ijkContextTag>();
}  // namespace ijk

namespace uv {
constexpr auto u = details::makeArg<0, details::contexts::uvContextTag>();
constexpr auto v = details::makeArg<1, details::contexts::uvContextTag>();
}  // namespace uv

}  // namespace injectx::args