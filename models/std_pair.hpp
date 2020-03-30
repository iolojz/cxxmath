//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_MODELS_STD_PAIR_HPP
#define CXXMATH_MODELS_STD_PAIR_HPP

#include "../concepts/product.hpp"

#include <utility>

namespace cxxmath {
struct std_pair_tag;

namespace impl {
template<class First, class Second>
struct tag_of<std::pair<First, Second>> {
	using type = std_pair_tag;
};
}

namespace model_std_pair {
struct first {
	template<class First, class Second>
	static constexpr auto apply( std::pair<First, Second> &&p ) {
		return p.first;
	}
	
	template<class First, class Second>
	static constexpr auto &apply( std::pair<First, Second> &p ) {
		return p.first;
	}
	
	template<class First, class Second>
	static constexpr const auto &apply( const std::pair<First, Second> &p ) {
		return p.first;
	}
};

struct second {
	template<class First, class Second>
	static constexpr auto apply( std::pair<First, Second> &&p ) {
		return p.second;
	}
	
	template<class First, class Second>
	static constexpr auto &apply( std::pair<First, Second> &p ) {
		return p.second;
	}
	
	template<class First, class Second>
	static constexpr const auto &apply( const std::pair<First, Second> &p ) {
		return p.second;
	}
};

using product = concepts::product<first, second>;
}

using std_pair_product = model_std_pair::product;

namespace impl {
template<>
struct default_product<std_pair_tag> {
	using type = model_std_pair::product;
};

template<> struct make_product<model_std_pair::product> {
	template<class Arg1, class Arg2>
	static constexpr auto apply( Arg1 &&arg1, Arg2 &&arg2 ) {
		return std::make_pair( std::forward<Arg1>( arg1 ), std::forward<Arg2>( arg2 ) );
	}
};

template<> struct make<std_pair_tag> : make_product<model_std_pair::product> {};
}

static constexpr auto make_pair = static_function_object<impl::make<std_pair_tag>>;
}

#endif //CXXMATH_MODELS_STD_PAIR_HPP
