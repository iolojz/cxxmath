//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_MODELS_PRODUCT_MONOID_HPP
#define CXXMATH_MODELS_PRODUCT_MONOID_HPP

#include "concepts/monoid.hpp"

namespace cxxmath
{
namespace model_product_monoid
{
namespace detail
{
template<class DispatchTag, class Monoid>
struct choose_monoid
{
	using type = std::conditional_t<
		std::is_void_v<Monoid>,
		default_monoid_t<DispatchTag>,
		Monoid
	>;
};
CXXMATH_DEFINE_TYPE_ALIAS_TEMPLATE(choose_monoid)
}

template<class Product, class FirstMonoid = void, class SecondMonoid = void>
struct is_abelian_monoid
{
private:
	static constexpr bool check_first( void )
	{
		if constexpr( std::is_void_v < FirstMonoid > ) {
			static_assert( Product::has_unique_first_tag,
						   "Cannot determine abelianness of defaulted first sub-monoid of product without unique first tag." );
			
			return default_monoid_t < typename Product::unique_first_tag > ::is_abelian_monoid();
		} else
			return FirstMonoid::is_abelian_monoid();
	}
	
	static constexpr bool check_second( void )
	{
		if constexpr( std::is_void_v < SecondMonoid > ) {
			static_assert( Product::has_unique_second_tag,
						   "Cannot determine abelianness of defaulted second sub-monoid of product without unique second tag." );
			
			return default_monoid_t < typename Product::unique_second_tag > ::is_abelian_monoid();
		} else
			return SecondMonoid::is_abelian_monoid();
	}
public:
	static constexpr bool apply( void )
	{
		return check_first() && check_second();
	}
};

template<class Product, class FirstMonoid = void, class SecondMonoid = void>
struct neutral_element
{
private:
	static constexpr decltype( auto ) neutral_first( void )
	{
		if constexpr( std::is_void_v < FirstMonoid > ) {
			static_assert( Product::has_unique_first_tag,
						   "Cannot construct neutral element of defaulted first sub-monoid of product without unique first tag." );
			
			return default_monoid_t < typename Product::unique_first_tag > ::neutral_element();
		} else
			return FirstMonoid::neutral_element();
	}
	
	static constexpr decltype( auto ) neutral_second( void )
	{
		if constexpr( std::is_void_v < SecondMonoid > ) {
			static_assert( Product::has_unique_second_tag,
						   "Cannot determine abelianness of defaulted second sub-monoid of product without unique second tag." );
			
			return default_monoid_t < typename Product::unique_second_tag > ::neutral_element();
		} else
			return SecondMonoid::neutral_element();
	}
public:
	template<class T = void>
	static constexpr auto apply( void )
	{
		return make_product<Product>::apply( FirstMonoid::neutral_element(), SecondMonoid::neutral_element());
	}
};

template<class Product, class FirstMonoid = void, class SecondMonoid = void>
struct compose
{
	template<class DispatchTag>
	static constexpr bool supports_tag( void )
	{ return true; }
	
	template<class Product1, class Product2>
	static constexpr auto apply( Product1 &&p1, Product2 &&p2 )
	{
		using first1_tag = tag_of_t<decltype( Product::first( std::forward<Product1>( p1 )))>;
		using first2_tag = tag_of_t<decltype( Product::first( std::forward<Product2>( p2 )))>;
		
		using second1_tag = tag_of_t<decltype( Product::second( std::forward<Product1>( p1 )))>;
		using second2_tag = tag_of_t<decltype( Product::second( std::forward<Product2>( p2 )))>;
		
		static_assert( std::is_same_v < first1_tag, first2_tag > , "Cannot compose objects of different tags." );
		static_assert( std::is_same_v < second1_tag, second2_tag > , "Cannot compose objects of different tags." );
		
		using first_monoid = detail::choose_monoid_t<first1_tag, FirstMonoid>;
		using second_monoid = detail::choose_monoid_t<second1_tag, SecondMonoid>;
		
		return make_product<Product>::apply( first_monoid::compose( Product::first( std::forward<Product1>( p1 )),
																	Product::first( std::forward<Product2>( p2 ))),
											 second_monoid::compose( Product::second( std::forward<Product1>( p1 )),
																	 Product::second( std::forward<Product2>( p2 ))));
	}
};

template<class Product, class FirstMonoid = void, class SecondMonoid = void>
struct compose_assign
{
	template<class DispatchTag>
	static constexpr bool supports_tag( void )
	{ return true; }
	
	template<class Product1, class Product2>
	static constexpr auto apply( Product1 &&p1, Product2 &&p2 )
	{
		using first1_tag = tag_of_t<decltype( Product::first( std::forward<Product1>( p1 )))>;
		using first2_tag = tag_of_t<decltype( Product::first( std::forward<Product2>( p2 )))>;
		
		using second1_tag = tag_of_t<decltype( Product::second( std::forward<Product1>( p1 )))>;
		using second2_tag = tag_of_t<decltype( Product::second( std::forward<Product2>( p2 )))>;
		
		static_assert( std::is_same_v < first1_tag, first2_tag > , "Cannot compose_assign objects of different tags." );
		static_assert( std::is_same_v < second1_tag, second2_tag > ,
					   "Cannot compose_assign objects of different tags." );
		
		using first_monoid = detail::choose_monoid_t<first1_tag, FirstMonoid>;
		using second_monoid = detail::choose_monoid_t<second1_tag, SecondMonoid>;
		
		auto &p11 = Product::first( p1 );
		auto &p12 = Product::first( p1 );
		
		p11 = first_monoid::compose_assign( Product::first( std::forward<Product2>( p1 )),
									  Product::first( std::forward<Product2>( p2 )));
		p12 = second_monoid::compose_assign( Product::second( std::forward<Product1>( p1 )),
									   Product::second( std::forward<Product2>( p2 )));
		return p1;
	}
};
}

template<class Product, class FirstMonoid = void, class SecondMonoid = void> using product_monoid = concepts::monoid<
	model_product_monoid::compose_assign<Product, FirstMonoid, SecondMonoid>,
	model_product_monoid::compose_product<Product, FirstMonoid, SecondMonoid>,
	model_product_monoid::neutral_element_product<Product, FirstMonoid, SecondMonoid>,
	model_product_monoid::is_abelian_monoid_product<Product, FirstMonoid, SecondMonoid>
>;

namespace impl
{
template<class DispatchTag>
struct default_monoid<DispatchTag, std::enable_if_t<has_default_product<DispatchTag>>>
{
using type = product_monoid<default_product_t<DispatchTag>>;
};
}
}

#endif //CXXMATH_MODELS_PRODUCT_MONOID_HPP
