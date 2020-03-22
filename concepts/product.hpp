//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_CONCEPTS_PRODUCT_HPP
#define CXXMATH_CONCEPTS_PRODUCT_HPP

#include "../models/function_object.hpp"

namespace cxxmath {
namespace concepts {
template<class First, class Second>
struct product {
	static constexpr auto first = static_function_object<First>;
	static constexpr auto second = static_function_object<Second>;
};

template<class> struct is_product: std::false_type {};
template<class First, class Second>
struct is_product<product<First, Second>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE( is_product )
}

template<class Type, class Product>
struct type_models_concept<Type, Product, std::enable_if_t<concepts::is_product_v<Product>>> {
	static constexpr bool value = (
		std::is_invocable_v<decltype(Product::first), Type> &&
		std::is_invocable_v<decltype(Product::second), Type>
	);
};

namespace impl {
template<class Product> struct make_product : unsupported_implementation {};
}

template<class Product> static constexpr auto make_product = static_function_object<impl::make_product<Product>>;

CXXMATH_DEFINE_CONCEPT( product )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( first, product )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( second, product )
}

#endif //CXXMATH_CONCEPTS_PRODUCT_HPP
