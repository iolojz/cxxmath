//
// Created by jayz on 28.09.19.
//

#ifndef CXXMATH_MODELS_PRODUCT_SET_HPP
#define CXXMATH_MODELS_PRODUCT_SET_HPP

namespace cxxmath
{
namespace model_product_set
{
namespace detail
{
template<class DispatchTag, class Set>
struct choose_set
{
	using type = Set;
};
template<class DispatchTag>
struct choose_set<DispatchTag, void>
{
	using type = default_set_t<DispatchTag>;
};
CXXMATH_DEFINE_TYPE_ALIAS_TEMPLATE( choose_set )
}

template<class Product, class FirstSet = void, class SecondSet = void>
struct equal
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
		
		static_assert( std::is_same_v<first1_tag, first2_tag>, "Cannot compare objects of different tags." );
		static_assert( std::is_same_v<second1_tag, second2_tag>, "Cannot compare objects of different tags." );
		
		using first_set = detail::choose_set_t<first1_tag, FirstSet>;
		using second_set = detail::choose_set_t<second1_tag, SecondSet>;
		
		return and_(
		first_set::equal( Product::first( std::forward<Product1>( p1 )), Product::first( std::forward<Product2>( p2 ))),
		second_set::equal( Product::second( std::forward<Product1>( p1 )),
						   Product::second( std::forward<Product2>( p2 ))));
	}
};
}

template<class Product> using product_set = concepts::set<model_product_set::equal<Product>>;

namespace impl
{
template<class DispatchTag>
struct default_set<DispatchTag, std::enable_if_t<has_default_product<DispatchTag>>>
{
	using type = product_set<default_product_t<DispatchTag>>;
};
}
}

#endif //CXXMATH_MODELS_PRODUCT_SET_HPP
