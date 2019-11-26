//
// Created by jayz on 16.11.19.
//

#ifndef CXXMATH_HELPERS_STD_VARIANT_HPP
#define CXXMATH_HELPERS_STD_VARIANT_HPP

#include "wrap_template_members.hpp"

namespace cxxmath {
template<class T> struct is_std_variant : std::false_type {};
template<class ...Alternatives> struct is_std_variant<std::variant<Alternatives...>> : std::true_type {};
CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_std_variant)

template<class T, class Variant>
struct variant_has_alternative;
CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(variant_has_alternative)

template<class T, class ...Alternatives>
struct variant_has_alternative<T, std::variant<Alternatives...>>
: public std::disjunction<std::is_same<T, Alternatives>...> {};

template<class T, class ...Alternatives>
static constexpr bool holds_alternative( const std::variant<Alternatives...> &v ) noexcept {
	if constexpr( variant_has_alternative_v<T, std::variant<Alternatives...>> )
		return std::holds_alternative<T>( v );
	else
		return false;
}

template<class Tag, class Variant> struct select_type_with_tag {};
CXXMATH_DEFINE_TYPE_ALIAS_TEMPLATE(select_type_with_tag)

namespace detail {
template<bool, class Tag, class T, class ...Alternatives> struct select_type_with_tag {
	using type = select_type_with_tag_t<Tag, std::variant<Alternatives...>>;
};

template<class Tag, class T, class ...Alternatives> struct select_type_with_tag<true, Tag, T, Alternatives...> {
	using type = T;
};
}

template<class Tag, class Type1, class ...Alternatives>
struct select_type_with_tag<Tag, std::variant<Type1, Alternatives...>>
: detail::select_type_with_tag<std::is_same_v<Tag, tag_of_t<Type1>>, Tag, Type1, Alternatives...> {};
}

#endif //CXXMATH_HELPERS_STD_VARIANT_HPP
