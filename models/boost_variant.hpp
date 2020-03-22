//
// Created by jayz on 22.03.20.
//

#ifndef CXXMATH_MODELS_BOOST_VARIANT_HPP
#define CXXMATH_MODELS_BOOST_VARIANT_HPP

#include <boost/variant.hpp>

namespace cxxmath {
struct boost_variant_tag {};

namespace model_boost_variant {
struct visit {
	template<class F, class BoostVariant>
	static constexpr decltype(auto) apply( F &&f, BoostVariant &&v ) {
		return boost::apply_visitor( std::forward<F>( f ), std::forward<BoostVariant>( v ) );
	}
};

struct get_with_predicate {
	template<class UnaryPredicate, class BoostVariant>
	static constexpr decltype(auto) apply( UnaryPredicate &&p, BoostVariant &&v ) {
		constexpr auto unwrapped_types = boost::hana::transform(
			boost::hana::to_tuple( typename std::decay_t<BoostVariant>::types{} ),
			boost::hana::metafunction<boost::unwrap_recursive>
		);
		constexpr auto types_satisfying_predicate = boost::hana::filter(
			unwrapped_types,
			[] ( auto &&t ) {
				return std::invoke_result_t<std::decay_t<UnaryPredicate>, typename decltype(+t)::type>{};
			}
		);
		
		static_assert( boost::hana::value( boost::hana::length( types_satisfying_predicate ) ) == 1 );
		using type = typename decltype(+boost::hana::front( types_satisfying_predicate ))::type;
		
		return boost::strict_get<type>( std::forward<BoostVariant>( v ) );
	}
};
}

namespace concepts {
using boost_variant = concepts::variant<
	model_boost_variant::visit,
	model_boost_variant::get_with_predicate
>;
}

namespace impl {
template<class ...Alternatives> struct tag_of<boost::variant<Alternatives...>> {
	using type = boost_variant_tag;
};

template<> struct default_variant<boost_variant_tag> {
	using type = concepts::boost_variant;
};
}
}

#endif //CXXMATH_MODELS_BOOST_VARIANT_HPP
