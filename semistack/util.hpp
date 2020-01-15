//
//  util.hpp
//  semistack
//
//  Created by Zeke Medley on 1/9/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#pragma once

#include <variant>
#include <optional>
#include <type_traits>

#include "value.hpp"
#include "instruction.hpp"

namespace vm {
namespace util {

// From: http://en.cppreference.com/w/cpp/utility/variant/visit
// For using lambdas with std::visit. For example:
// std::visit(overloaded {
//              [](std::string f) { ... },
//              [](float f) { ... }
//              }, v);
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template <class T>
using optional_ref = std::optional<std::reference_wrapper<T>>;

// Variant of std::get that searches nested variants for the type. If a variant
// currently holding the requested type does not exist, returns std::nullopt.
// Will never throw std::bad_variant_access.
template<class T, class... Vts>
optional_ref<T> get(std::variant<Vts...>& v);

inline Instruction make_instruction(InstType t) {return {std::move(t), std::nullopt}; }

template<class>
struct IsVariant: std::false_type {};

template<class... Ts>
struct IsVariant<std::variant<Ts...>>: std::true_type {};

template<typename T, typename V>
struct isVariantMember;

template<typename T, typename... Vts>
struct isVariantMember<T, std::variant<Vts...>>
  : public std::disjunction<std::is_same<T, Vts>...> {};

template<class T>
constexpr bool is_variant() { return IsVariant<T>::value; }

template<class T, class V>
constexpr bool is_variant_member() { return isVariantMember<T, V>::value; }

template<typename T, typename... Ts>
constexpr bool contains()
{
    return std::disjunction_v<std::is_same<T, Ts>...>;
}

template <class T, class... Vts>
inline typename std::enable_if<contains<T, Vts...>(), optional_ref<T>>::type
get_if_contains(std::variant<Vts...>& v)
{
    if ( ! std::holds_alternative<T>(v) ) return std::nullopt;
    return std::get<T>(v);
}

template <class T, class... Vts>
inline typename std::enable_if<(not contains<T, Vts...>()), optional_ref<T>>::type
get_if_contains(std::variant<Vts...>& v)
{
    return std::nullopt;
}

template<class T, class V>
inline typename std::enable_if<is_variant<V>(), optional_ref<T>>::type
get_helper(V& v)
{
    return util::get<T>(v);
}

template<class T, class V>
inline typename std::enable_if<(not is_variant<V>()), optional_ref<T>>::type
get_helper(V& v)
{
    return std::nullopt;
}

// Variant of std::get that searches nested variants for the type. If a variant
// currently holding the requested type does not exist, returns std::nullopt.
template<class T, class... Vts>
inline optional_ref<T> get(std::variant<Vts...>& v)
{
    auto val{std::move(get_if_contains<T>(v))};

    return val ? val : std::visit([](auto& w) {
        return get_helper<T>(w);
    }, v);
}

// Specialization for if immediate is a Value or a Value can be made from it.
template<class T>
inline typename std::enable_if<std::is_constructible_v<Value, T>, Instruction>::type
make_instruction(InstType&& t, T&& immediate)
{
    return {std::forward<InstType>(t), std::forward<T>(immediate)};
}

}
}
