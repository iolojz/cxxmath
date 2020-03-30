//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_MODELS_PRODUCT_MONOID_HPP
#define CXXMATH_MODELS_PRODUCT_MONOID_HPP

#include "../concepts/monoid.hpp"

namespace cxxmath {
namespace model_product_monoid {
template<class Product, class FirstMonoid, class SecondMonoid>
struct is_abelian_monoid {
	static constexpr bool apply( void ) {
		return FirstMonoid::is_abelian_monoid() && SecondMonoid::is_abelian_monoid();
	}
};

template<class Product, class FirstMonoid, class SecondMonoid>
struct neutral_element {
	static constexpr auto apply( void ) {
		return make_product<Product>( FirstMonoid::neutral_element(), SecondMonoid::neutral_element() );
	}
};

template<class Product, class FirstMonoid, class SecondMonoid>
struct compose {
	template<class Product1, class Product2>
	static constexpr auto apply( Product1 &&p1, Product2 &&p2 ) {
		using first1_tag = tag_of_t<decltype( Product::first( std::forward<Product1>( p1 ) ) )>;
		using first2_tag = tag_of_t<decltype( Product::first( std::forward<Product2>( p2 ) ) )>;
		
		using second1_tag = tag_of_t<decltype( Product::second( std::forward<Product1>( p1 ) ) )>;
		using second2_tag = tag_of_t<decltype( Product::second( std::forward<Product2>( p2 ) ) )>;
		
		static_assert( std::is_same_v<first1_tag, first2_tag>, "Cannot compose objects of different tags." );
		static_assert( std::is_same_v<second1_tag, second2_tag>, "Cannot compose objects of different tags." );
		
		return make_product<Product>(
			FirstMonoid::compose(
				Product::first( std::forward<Product1>( p1 ) ),
				Product::first( std::forward<Product2>( p2 ) )
			),
			SecondMonoid::compose(
				Product::second( std::forward<Product1>( p1 ) ),
				Product::second( std::forward<Product2>( p2 ) )
			)
		);
	}
};

template<class Product, class FirstMonoid, class SecondMonoid>
struct compose_assign {
	template<class Product1, class Product2>
	static constexpr auto apply( Product1 &&p1, Product2 &&p2 ) {
		using first1_tag = tag_of_t<decltype( Product::first( std::forward<Product1>( p1 ) ) )>;
		using first2_tag = tag_of_t<decltype( Product::first( std::forward<Product2>( p2 ) ) )>;
		
		using second1_tag = tag_of_t<decltype( Product::second( std::forward<Product1>( p1 ) ) )>;
		using second2_tag = tag_of_t<decltype( Product::second( std::forward<Product2>( p2 ) ) )>;
		
		static_assert( std::is_same_v<first1_tag, first2_tag>, "Cannot compose_assign objects of different tags." );
		static_assert(
			std::is_same_v<second1_tag, second2_tag>,
			"Cannot compose_assign objects of different tags."
		);
		
		auto &p11 = Product::first( p1 );
		auto &p12 = Product::second( p1 );
		
		p11 = FirstMonoid::compose_assign(
			Product::first( std::forward<Product2>( p1 ) ),
			Product::first( std::forward<Product2>( p2 ) )
		);
		p12 = SecondMonoid::compose_assign(
			Product::second( std::forward<Product1>( p1 ) ),
			Product::second( std::forward<Product2>( p2 ) )
		);
		return p1;
	}
};
}

template<class Product, class FirstMonoid, class SecondMonoid> using product_monoid = concepts::monoid<
	model_product_monoid::compose_assign<Product, FirstMonoid, SecondMonoid>,
	model_product_monoid::compose<Product, FirstMonoid, SecondMonoid>,
	model_product_monoid::neutral_element<Product, FirstMonoid, SecondMonoid>,
	model_product_monoid::is_abelian_monoid<Product, FirstMonoid, SecondMonoid>
>;

namespace impl {
template<class DispatchTag1, class DispatchTag2>
struct default_monoid<
	multitag<DispatchTag1, DispatchTag2>,
	std::enable_if_t<
		has_default_product_v<multitag<DispatchTag1, DispatchTag2>> &&
		has_default_monoid_v<DispatchTag1> &&
		has_default_monoid_v<DispatchTag2>
	>
> {
	using type = product_monoid<
	    default_product_t<multitag<DispatchTag1, DispatchTag2>>,
		default_monoid_t<DispatchTag1>,
		default_monoid_t<DispatchTag2>
	>;
};
}
}

#endif //CXXMATH_MODELS_PRODUCT_MONOID_HPP
