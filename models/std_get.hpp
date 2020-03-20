//
// Created by jayz on 03.10.19.
//

#ifndef CXXMATH_MODELS_STD_GET_HPP
#define CXXMATH_MODELS_STD_GET_HPP

#include "../concepts/product.hpp"

#include "std_pair.hpp"

namespace cxxmath {
namespace model_std_get {
struct std_get_0 {
	template<class Arg> static constexpr auto apply( Arg &&arg )
	/* Make us SFINAE-friendly */ -> decltype(std::get<0>( std::forward<Arg>( arg ) )) {
		return std::get<0>( std::forward<Arg>( arg ) );
	}
};

struct std_get_1 {
	template<class Arg> static constexpr auto apply( Arg &&arg )
	/* Make us SFINAE-friendly */ -> decltype(std::get<1>( std::forward<Arg>( arg ) )) {
		return std::get<1>( std::forward<Arg>( arg ) );
	}
};
}

using std_get_product = concepts::product<model_std_get::std_get_0, model_std_get::std_get_1>;

namespace impl {
template<>
struct make_product<std_get_product> {
	template<class Arg1, class Arg2>
	static constexpr decltype(auto) apply( Arg1 &&arg1, Arg2 &&arg2 ) {
		return std::make_pair( std::forward<Arg1>( arg1 ), std::forward<Arg2>( arg2 ) );
	}
};
}
}

#endif //CXXMATH_MODELS_STD_GET_HPP
