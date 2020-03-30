//
// Created by jayz on 30.03.20.
//

#ifndef CXXMATH_MODELS_PRODUCT_COMPARABLE_HPP
#define CXXMATH_MODELS_PRODUCT_COMPARABLE_HPP

#include "../concepts/comparable.hpp"

namespace cxxmath {
namespace model_product_comparable {
template<class Product, class FirstComparable, class SecondComparable>
struct equal {
	template<class Product1, class Product2>
	static constexpr auto apply( Product1 &&p1, Product2 &&p2 ) {
		using first1_tag = tag_of_t<decltype( Product::first( std::forward<Product1>( p1 ) ) )>;
		using first2_tag = tag_of_t<decltype( Product::first( std::forward<Product2>( p2 ) ) )>;
		
		using second1_tag = tag_of_t<decltype( Product::second( std::forward<Product1>( p1 ) ) )>;
		using second2_tag = tag_of_t<decltype( Product::second( std::forward<Product2>( p2 ) ) )>;
		
		static_assert( std::is_same_v<first1_tag, first2_tag>, "Cannot compare objects of different tags." );
		static_assert( std::is_same_v<second1_tag, second2_tag>, "Cannot compare objects of different tags." );
		
		return (
			FirstComparable::equal(
				Product::first( std::forward<Product1>( p1 ) ),
				Product::first( std::forward<Product2>( p2 ) )
			) &&
			SecondComparable::equal(
				Product::second( std::forward<Product1>( p1 ) ),
				Product::second( std::forward<Product2>( p2 ) )
			)
		);
	}
};
}

template<class Product, class FirstMonoid, class SecondMonoid>
using product_comparable = concepts::comparable<
	model_product_comparable::equal<Product, FirstMonoid, SecondMonoid>
>;

namespace impl {
template<class DispatchTag1, class DispatchTag2>
struct default_comparable<
	multitag<DispatchTag1, DispatchTag2>,
	std::enable_if_t<
		has_default_product_v<multitag<DispatchTag1, DispatchTag2>> &&
		has_default_comparable_v<DispatchTag1> &&
		has_default_comparable_v<DispatchTag2>
	>
> {
	using type = product_monoid<
		default_product_t<multitag<DispatchTag1, DispatchTag2>>,
		default_comparable_t<DispatchTag1>,
		default_comparable_t<DispatchTag2>
	>;
};
}
}

#endif //CXXMATH_MODELS_PRODUCT_COMPARABLE_HPP
