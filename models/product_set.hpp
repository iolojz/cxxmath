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

template<class Product, class FirstSet = void, class SecondSet = void>
using product_set = concepts::set<model_product_set::equal<Product, FirstSet, SecondSet>>;

namespace detail {
template<class DispatchTag, bool> class has_default_product_set {
	using product = default_product_t<DispatchTag>;
public:
	static constexpr bool value = std::conditional_t<
		product::has_unique_first_tag && product::has_unique_second_tag,
		std::bool_constant<has_default_set_v<typename product::unique_first_tag> && has_default_set_v<typename product::unique_first_tag>>,
		std::false_type
	>::value;
};

template<class DispatchTag> class has_default_product_set<DispatchTag, false> : public std::false_type {};
}

template<class DispatchTag> struct has_default_product_set
: public detail::has_default_product_set<DispatchTag, has_default_product_v<DispatchTag>> {};
CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(has_default_product_set)

namespace impl
{

template<class DispatchTag>
struct default_set<DispatchTag, std::enable_if_t<has_default_product_set_v<DispatchTag>>>
{
	using type = product_set<default_product_t<DispatchTag>>;
};
}
}

#endif //CXXMATH_MODELS_PRODUCT_SET_HPP
