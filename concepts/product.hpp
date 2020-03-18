//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_CONCEPTS_PRODUCT_HPP
#define CXXMATH_CONCEPTS_PRODUCT_HPP

#include "../models/function_object.hpp"

namespace cxxmath {
namespace concepts {
template<class First, class Second, class MakeProduct>
struct product {
	static constexpr auto first = function_object_v<First>;
	static constexpr auto second = function_object_v<Second>;
};

template<class> struct is_product: std::false_type {};
template<class First, class Second>
struct is_product<product<First, Second>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE( is_product )
}

template<class Type, class Product>
struct type_models_concept<Type, Product, std::enable_if_t<concepts::is_product_v<Product>>> {
	static constexpr bool value = (
		CXXMATH_IS_VALID( Product::first, std::declval<Type>() ) &&
		CXXMATH_IS_VALID( Product::second, std::declval<Type>() )
	);
};

namespace impl {
template<class Product> struct make_product : unsupported_implementation;
}

template<class Product> static constexpr auto make_product = function_object_v<impl::make_product<Product>>;

CXXMATH_DEFINE_CONCEPT( product )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( first, product )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( second, product )
}

#endif //CXXMATH_CONCEPTS_PRODUCT_HPP
