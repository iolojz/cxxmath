//
// Created by jayz on 21.03.20.
//

#ifndef CXXMATH_HELPERS_BOOST_VARIANT_HPP
#define CXXMATH_HELPERS_BOOST_VARIANT_HPP

#include <boost/hana/ext/boost/mpl/list.hpp>
#include <boost/hana/ext/boost/mpl/vector.hpp>
#include <boost/hana/ext/std/integral_constant.hpp>

namespace cxxmath {
template<class> struct is_boost_variant : std::false_type {};
template<class ...Alternatives> struct is_boost_variant<boost::variant<Alternatives...>> : std::true_type {};
CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_boost_variant)

namespace detail {
template<class BoostVariant, class ...Args>
class constructible_types {
	static_assert( is_boost_variant_v<BoostVariant> );
	static constexpr auto unwrapped_typpes = boost::hana::transform(
		boost::hana::to_tuple( typename BoostVariant::types{} ),
		boost::hana::metafunction<boost::unwrap_recursive>
	);
public:
	static constexpr auto value = boost::hana::filter(
		unwrapped_typpes,
		[] ( auto &&t ) {
			return boost::hana::bool_c<std::is_constructible_v<typename decltype(+t)::type, Args...>>;
		}
	);
};

template<bool uniquely_constructible, class BoostVariant, class ...Args>
struct unique_constructible_alternative {};

template<class BoostVariant, class ...Args>
struct unique_constructible_alternative<true, BoostVariant, Args...> {
	using type = typename decltype(+boost::hana::front( constructible_types<BoostVariant, Args...>::value ))::type;
};
}

template<class BoostVariant, class ...Args> struct has_unique_constructible_alternative {
	static constexpr bool value = (
		boost::hana::value(boost::hana::length(detail::constructible_types<BoostVariant, Args...>::value)) == 1
	);
};
CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(has_unique_constructible_alternative)

template<class BoostVariant, class ...Args> struct unique_constructible_alternative
: detail::unique_constructible_alternative<
	has_unique_constructible_alternative_v<BoostVariant, Args...>,
	BoostVariant,
	Args...
> {};
CXXMATH_DEFINE_TYPE_ALIAS_TEMPLATE(unique_constructible_alternative)
}

#endif //CXXMATH_HELPERS_BOOST_VARIANT_HPP
