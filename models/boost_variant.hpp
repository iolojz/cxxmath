//
// Created by jayz on 22.03.20.
//

#ifndef CXXMATH_MODELS_BOOST_VARIANT_HPP
#define CXXMATH_MODELS_BOOST_VARIANT_HPP

#include <boost/variant.hpp>

#include "../concepts/variant.hpp"

namespace cxxmath {
struct boost_variant_tag {};

namespace model_boost_variant {
struct types {
	template<class BoostVariant>
	static constexpr decltype(auto) apply( BoostVariant && ) {
		return boost::hana::to_tuple( typename std::decay_t<BoostVariant>::types{} );
	}
};

struct visit {
	template<class F, class BoostVariant>
	static constexpr decltype(auto) apply( F &&f, BoostVariant &&v ) {
		return boost::apply_visitor( std::forward<F>( f ), std::forward<BoostVariant>( v ) );
	}
};
}

namespace concepts {
using boost_variant = concepts::variant<
	model_boost_variant::types,
	model_boost_variant::visit
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
